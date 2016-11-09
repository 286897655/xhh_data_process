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
			// �������� ˵�������������
			throw std::runtime_error("���ݴ��������⡢������û�Ӷ�������");
		}
	}

	void WindDataRepair::Start(const std::string& incsvFile, const std::string& outcsvFile){

		std::future<bool> readfile = std::async(std::launch::async, [&]()->bool
		{
			// �����ڶ�ȡ���ݵ�����
			ReadFileDataToQueue(incsvFile);

			return true;
		});

		std::future<bool> analysis = std::async(std::launch::async, [&]()->bool
		{
			// ���������е�����
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
			// ������ݵ��ļ�
			// ������csv�ļ�
			std::ofstream ofs(outcsvFile, std::ios_base::out);
			if (!ofs.is_open())
			{
				printf_s("���ܴ��ļ�\n");
				throw std::runtime_error("can't open file");
			}

			// ��û�ж�ȡ��ͷ��ʱ��ȴ�
			while (m_fileheader.size() == 0)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}

			// ���ͷ
			for each(const auto& var in m_fileheader)
			{
				ofs << var << ",";
			}
			ofs << '\n';

			// ��������������
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

			printf_s("����ļ�����\n");
		});

		outfile.join();
		printf_s("���̰߳洦�����");
	}

	void WindDataRepair::OutQueueToFile(std::ofstream* ofs, const QueueWindData& curdata)
	{
		std::tm* firstday = JGG::TimePointTotm(curdata.begin()->currTime);
		char DayFormat[128];
		sprintf_s(DayFormat, "%d/%d/%d", firstday->tm_year + 1900, firstday->tm_mon, firstday->tm_mday);
		std::string  curDay(DayFormat);
		printf_s("��ǰ�������:%s\n", curDay.c_str());

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

		printf_s("��ǰ������������:%s\n", curDay.c_str());
		printf_s("��ǰ������������:%d\n", curdata.size());


		// Start Time 
		const static std::string starttime = "00:50:10";
		// End Time
		const static std::string endtime = "23:50:00";

		JGG::TimePoint StartTime_Point = JGG::StringToT(curDay + "+" + starttime, "%d/%d/%d+%d:%d:%d");

		JGG::TimePoint EndTime_Point = JGG::StringToT(curDay + "+" + endtime, "%d/%d/%d+%d:%d:%d");

		// һ�����������Ƕ�ȡ��һ�������
		QueueWindData  newdata;

		// ��ͷ
		while (StartTime_Point < curdata.begin()->currTime)
		{
			WindDataOnTime newtimedata;
			newtimedata.currTime = StartTime_Point;

			newtimedata.restdata = std::vector<std::string>(m_fileheader.size() - 2, "9999");//-2ȥ�����ں�ʱ��

			// 10 seconds
			StartTime_Point += std::chrono::milliseconds(10000);

			newdata.insert(newtimedata);
		}

		// ����
		for each (const auto& windtime in curdata)
		{
			// ����ʼʱ��С������ֱ�ӱ���
			auto period1 = windtime.currTime - StartTime_Point;
			if (period1.count() <= 0)
			{
				newdata.insert(windtime);
				continue;
			}
			// ����ֹʱ��������ֱ�ӱ���
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

					newtimedata.restdata = std::vector<std::string>(m_fileheader.size() - 2, "9999");//-2ȥ�����ں�ʱ��

					newdata.insert(newtimedata);
				}
			}
		}

		// ��β
		while (StartTime_Point < EndTime_Point)
		{
			StartTime_Point += std::chrono::milliseconds(10000);
			WindDataOnTime newtimedata;
			newtimedata.currTime = StartTime_Point;

			newtimedata.restdata = std::vector<std::string>(m_fileheader.size() - 2, "9999");//-2ȥ�����ں�ʱ��

			newdata.insert(newtimedata);
		}

		printf_s("����%s����,������Ϊ:%d\n", curDay.c_str(), newdata.size());

		m_analysistofileout.push(newdata);
	}

	void WindDataRepair::ReadFileDataToQueue(const std::string& dataFile)
	{
		// ������csv�ļ�
		std::ifstream ifs(dataFile, std::ios_base::in);
		if (!ifs.is_open())
		{
			printf_s("���ܴ��ļ�\n");
			throw std::runtime_error("open file error");
		}

		printf_s("��ʼ��ȡ����\n");

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

				// ���֮ǰ������
				push_data.clear();
				break;
			}
			printf_s("��ʼ��ȡ��:------%d-----��\n", ++i);

			std::string readstr;
			buffer >> readstr;
			std::vector<std::string> splited = JGG::splitstring(readstr);

			if (splited.empty())
				continue;
			//��һ��Ϊ�ļ�ͷ
			if (1 == i)
			{
				m_fileheader = splited;
				continue;
			}

			// ���ڱ仯����һ�������
			if (splited[0] != lastday)
			{
				lastday = splited[0];
				if (push_data.size() > 0)
				{
					m_filereadtoanalysis.push(push_data);

					// ���֮ǰ������
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
		printf_s("��ȡ�ļ�����\n");
	}
}
