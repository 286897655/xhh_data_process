#include "WindDataRepair.h"
#include <future>
#include <fstream>
#include <sstream>

#include "stringutil.h"

namespace JGG
{
	WindDataRepair::WindDataRepair()
	{
		
	}
	WindDataRepair::~WindDataRepair()
	{
		if (m_filereadtoanalysis.size() > 0 || m_analysistofileout.size() > 0)
		{
			// 还有数据 说明处理的有问题
			throw std::runtime_error("数据处理有问题、有数据没从队列清理");
		}
	}

	void WindDataRepair::Start(const std::string& incsvFile, const std::string& outcsvFile){

		std::future<bool> readfile = std::async(std::launch::async, [&]()->bool
		{
			// 按日期读取数据到队列
			ReadFileDataToQueue(incsvFile);

			return true;
		});

		std::future<bool> analysis = std::async(std::launch::async, [&]()->bool
		{
			// 解析队列中的数据
			for (;;)
			{
				if (m_filereadtoanalysis.size() == 0)
				{
					if (readfile._Is_ready())
					{
						break;
					}
					else
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						continue;
					}
				}

				AnalysisDataToQueue(m_filereadtoanalysis.front());
				m_filereadtoanalysis.pop();
			}

			return true;
		});

		std::thread outfile([&]()->void{
			// 输出数据到文件
			// 用流打开csv文件
			std::ofstream ofs(outcsvFile, std::ios_base::out);
			if (!ofs.is_open())
			{
				printf_s("不能打开文件\n");
				throw std::runtime_error("can't open file");
			}

			// 还没有读取到头的时候等待
			while (m_fileheader.size() == 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			// 输出头
			for each(const auto& var in m_fileheader)
			{
				ofs << var << ",";
			}
			ofs << '\n';

			// 解析队列中数据
			for (;;)
			{
				if (m_analysistofileout.size() == 0)
				{
					if (analysis._Is_ready())
					{
						break;
					}
					else
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(1000));
						continue;
					}
				}
				OutQueueToFile(&ofs, m_analysistofileout.front());
				m_analysistofileout.pop();
			}

			ofs.flush();
			ofs.close();

			printf_s("输出文件结束\n");
		});

		outfile.join();
		printf_s("多线程版处理结束");
	}

	void WindDataRepair::OutQueueToFile(std::ofstream* ofs, const QueueWindData& curdata)
	{
		std::tm* firstday = JGG::TimePointTotm(curdata.begin()->currTime);
		char DayFormat[128];
		sprintf_s(DayFormat, "%d/%d/%d", firstday->tm_year + 1900, firstday->tm_mon, firstday->tm_mday);
		std::string  curDay(DayFormat);
		printf_s("当前输出日期:%s\n", curDay.c_str());

		for each(const auto& windtime in curdata)
		{
			std::tm* nowtime = JGG::TimePointTotm(windtime.currTime);
			char TimeFormat[128];
			sprintf_s(TimeFormat, "%d:%d:%d", nowtime->tm_hour, nowtime->tm_min, nowtime->tm_sec);

			std::string NewTime(TimeFormat);
			
			*ofs << curDay << "," << NewTime << ",";

			for each(const auto& restdata in windtime.restdata)
			{
				*ofs << restdata << ",";
			}

			*ofs << '\n';
		}
	}

	void WindDataRepair::AnalysisDataToQueue(const QueueWindData& curdata)
	{
		std::tm* nowtime = JGG::TimePointTotm(curdata.begin()->currTime);

		char DayFormat[128];
		sprintf_s(DayFormat, "%d/%d/%d", nowtime->tm_year + 1900, nowtime->tm_mon, nowtime->tm_mday);

		std::string curDay(DayFormat);

		printf_s("当前处理数据日期:%s\n", curDay.c_str());
		printf_s("当前日期数据总数:%d\n", curdata.size());


		// Start Time 
		const static std::string starttime = "00:50:10";
		// End Time
		const static std::string endtime = "23:50:00";

		JGG::TimePoint StartTime_Point = JGG::StringToT(curDay + "+" + starttime, "%d/%d/%d+%d:%d:%d");

		JGG::TimePoint EndTime_Point = JGG::StringToT(curDay + "+" + endtime, "%d/%d/%d+%d:%d:%d");

		// 一个队列数据是读取的一天的数据
		QueueWindData  newdata;

		// 补头
		while (StartTime_Point < curdata.begin()->currTime)
		{
			WindDataOnTime newtimedata;
			newtimedata.currTime = StartTime_Point;

			newtimedata.restdata = std::vector<std::string>(m_fileheader.size() - 2, "9999");//-2去掉日期和时间

			// 10 seconds
			StartTime_Point += std::chrono::milliseconds(10000);

			newdata.insert(newtimedata);
		}

		// 补中
		for each (const auto& windtime in curdata)
		{
			// 比起始时间小的数据直接保存
			auto period1 = windtime.currTime - StartTime_Point;
			if (period1.count() <= 0)
			{
				newdata.insert(windtime);
				continue;
			}
			// 比终止时间大的数据直接保存
			auto period2 = windtime.currTime - EndTime_Point;
			if (period2.count() >= 0)
			{
				StartTime_Point += std::chrono::milliseconds(10000);
				newdata.insert(windtime);
				continue;
			}

			while (true)
			{
				StartTime_Point += std::chrono::milliseconds(10000);
				if (StartTime_Point == windtime.currTime)
				{
					newdata.insert(windtime);
					break;
				}
				else
				{
					WindDataOnTime newtimedata;
					newtimedata.currTime = StartTime_Point;

					newtimedata.restdata = std::vector<std::string>(m_fileheader.size() - 2, "9999");//-2去掉日期和时间

					newdata.insert(newtimedata);
				}
			}
		}

		// 补尾
		while (StartTime_Point < EndTime_Point)
		{
			StartTime_Point += std::chrono::milliseconds(10000);
			WindDataOnTime newtimedata;
			newtimedata.currTime = StartTime_Point;

			newtimedata.restdata = std::vector<std::string>(m_fileheader.size() - 2, "9999");//-2去掉日期和时间

			newdata.insert(newtimedata);
		}

		printf_s("补完%s数据,总数据为:%d\n", curDay.c_str(), newdata.size());

		m_analysistofileout.push(newdata);
	}

	void WindDataRepair::ReadFileDataToQueue(const std::string& dataFile)
	{
		// 用流打开csv文件
		std::ifstream ifs(dataFile, std::ios_base::in);
		if (!ifs.is_open())
		{
			printf_s("不能打开文件\n");
			throw std::runtime_error("open file error");
		}

		printf_s("开始读取数据\n");

		// read all in memery
		std::stringstream buffer;
		buffer << ifs.rdbuf();
		ifs.close();
		std::string lastday("");
		QueueWindData push_data;
		int i = 0;
		while (1)
		{
			if (buffer.eof())
			{
				m_filereadtoanalysis.push(push_data);

				// 清除之前的数据
				push_data.clear();
				break;
			}
			printf_s("开始读取第:------%d-----行\n", ++i);

			std::string readstr;
			buffer >> readstr;
			std::vector<std::string> splited = JGG::splitstring(readstr);

			if (splited.empty())
				continue;
			//第一行为文件头
			if (1 == i)
			{
				m_fileheader = splited;
				continue;
			}

			// 日期变化读新一天的数据
			if (splited[0] != lastday)
			{
				lastday = splited[0];
				if (push_data.size() > 0)
				{
					m_filereadtoanalysis.push(push_data);

					// 清除之前的数据
					push_data.clear();
				}
			}
			WindDataOnTime tmpdata;
			tmpdata.currTime = JGG::StringToT(splited[0] + "+" + splited[1], "%d/%d/%d+%d:%d:%d");

			for (std::vector<std::string>::const_iterator it = splited.begin() + 2; it != splited.end(); it++)
			{
				tmpdata.restdata.push_back(*it);
			}

			push_data.insert(tmpdata);
		}
		printf_s("读取文件结束\n");
	}
}
