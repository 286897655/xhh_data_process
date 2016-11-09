// xhhshujuchuli.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include "WindDataRepair.h"
#include "timeutil.h"
#include "stringutil.h"

struct WindTime
{
	JGG::TimePoint currTime;

	std::vector<std::string> restdata;
};

static std::vector<std::string> Head;


void OutDataToCSV(const std::string& csvfile, const std::map<std::string, std::vector<WindTime>>& AllData)
{
	// 用流打开csv文件
	std::ofstream ofs(csvfile, std::ios_base::out);

	if (!ofs.is_open())
	{
		printf_s("不能打开文件\n");
		throw std::runtime_error("can't open file");
	}
	printf_s("开始输出数据\n");

	// 输出头
	for each(const auto& var in Head)
	{
		ofs << var << ",";
	}
	ofs << '\n';

	for each(const auto& var in AllData)
	{
		printf_s("当前输出日期:%s\n", var.first.c_str());
		
		for  each(const auto& windtime in var.second)
		{
			std::tm* nowtime = JGG::TimePointTotm(windtime.currTime);
			
			char TimeFormat[128];
			sprintf_s(TimeFormat, "%d:%d:%d", nowtime->tm_hour, nowtime->tm_min, nowtime->tm_sec);

			std::string NewTime(TimeFormat);

			ofs << var.first << "," << NewTime << ",";

			for each(const auto& restdata in windtime.restdata)
			{
				ofs << restdata << ",";
			}

			ofs << '\n';
		}
	}

	ofs.flush();
	ofs.close();

	printf_s("输出文件结束\n");
}

std::map<std::string, std::vector<WindTime>> ReadDataByCSV(const std::string& csvfile)
{
	// 用流打开csv文件
	std::ifstream ifs(csvfile, std::ios_base::in);
	if (!ifs.is_open())
	{
		printf_s("不能打开文件\n");
		throw std::runtime_error("open file error");
	}
	printf_s("开始读取数据\n");
	std::map<std::string, std::vector<WindTime>> allDataByDate;
	int i = 0;
	// read all in memery
	std::stringstream buffer;
	buffer << ifs.rdbuf();
	ifs.close();
	while (!buffer.eof())
	{
		printf_s("开始读取第%d行\n", ++i);
		std::string readstr;
		buffer >> readstr;

		std::vector<std::string> splited = JGG::splitstring(readstr);

		if (splited.empty())
			continue;
		if (1 == i)
		{
			Head = splited;
			continue;
		}	

		WindTime tmpdata;
		tmpdata.currTime = JGG::StringToT(splited[0] + "+" + splited[1], "%d/%d/%d+%d:%d:%d");

		for (std::vector<std::string>::const_iterator it = splited.begin() + 2; it != splited.end(); it++)
		{
			tmpdata.restdata.push_back(*it);
		}

		allDataByDate[splited[0]].push_back(tmpdata);

	}
	printf_s("读取文件结束\n");
	return allDataByDate;
}

void OldSingleThreadVersion(const std::string& incsvFile,const std::string& outcsvFile)
{
	std::map<std::string, std::vector<WindTime>> allDataByDate = ReadDataByCSV(incsvFile);

	const std::string startDate = allDataByDate.begin()->first;
	const std::string endDate = allDataByDate.rbegin()->first;

	std::map<std::string, std::vector<WindTime>> NewAllData;

	for each (const auto& var in allDataByDate)
	{
		printf_s("文件数据开始日期:%s---文件数据结束日期:%s---\n", startDate.c_str(), endDate.c_str());
		printf_s("当前处理数据日期:%s\n", var.first.c_str());
		printf_s("当前日期数据总数:%d\n", var.second.size());

		std::vector<WindTime> dayData = var.second;

		std::sort(dayData.begin(), dayData.end(), [](const WindTime& first, const WindTime& second)->int{
			return (first.currTime <= second.currTime) ? 1 : 0;
		});

		// Start Time 
		const std::string starttime = "00:50:10";
		// End Time
		const std::string endtime = "23:50:00";

		JGG::TimePoint StartTime_Point = JGG::StringToT(var.first + "+" + starttime, "%d/%d/%d+%d:%d:%d");

		JGG::TimePoint EndTime_Point = JGG::StringToT(var.first + "+" + endtime, "%d/%d/%d+%d:%d:%d");

		std::vector<WindTime> newData;

		// 补头
		while (StartTime_Point < dayData[0].currTime)
		{
			WindTime newtimedata;
			newtimedata.currTime = StartTime_Point;

			newtimedata.restdata = std::vector<std::string>(Head.size() - 2, "9999");//-2去掉日期和时间

			StartTime_Point += std::chrono::milliseconds(10000);

			newData.push_back(newtimedata);
		}
		// 补中
		for each (const auto& windtime in dayData)
		{
			// 比起始时间小的数据直接保存
			auto period1 = windtime.currTime - StartTime_Point;
			if (period1.count() <= 0)
			{
				newData.push_back(windtime);
				continue;
			}
			// 比终止时间大的数据直接保存
			auto period2 = windtime.currTime - EndTime_Point;
			if (period2.count() >= 0)
			{
				StartTime_Point += std::chrono::milliseconds(10000);
				newData.push_back(windtime);
				continue;
			}

			while (true)
			{
				StartTime_Point += std::chrono::milliseconds(10000);
				if (StartTime_Point == windtime.currTime)
				{
					newData.push_back(windtime);
					break;
				}
				else
				{
					WindTime newtimedata;
					newtimedata.currTime = StartTime_Point;

					newtimedata.restdata = std::vector<std::string>(Head.size() - 2, "9999");//-2去掉日期和时间

					newData.push_back(newtimedata);
				}
			}
		}

		// 补尾
		while (StartTime_Point < EndTime_Point)
		{
			StartTime_Point += std::chrono::milliseconds(10000);
			WindTime newtimedata;
			newtimedata.currTime = StartTime_Point;

			newtimedata.restdata = std::vector<std::string>(Head.size() - 2, "9999");//-2去掉日期和时间

			newData.push_back(newtimedata);
		}

		printf_s("补完%s数据,总数据为:%d\n", var.first.c_str(), newData.size());

		NewAllData[var.first] = newData;
	}
	// 输出文件到csv文件
	std::cout << "数据补充结束---" << "输出数据到csv文件" << std::endl;

	OutDataToCSV(outcsvFile, NewAllData);
}



int main(int argc, char* argv[])
{
	std::clock_t starttime = std::clock();

	JGG::WindDataRepair *dataRepair = new JGG::WindDataRepair();

	dataRepair->Start("winddata.csv", "multithread.csv");

	delete dataRepair;

	std::clock_t endtime1 = std::clock();

	/*OldSingleThreadVersion("winddata.csv", "singthread.csv");
	
	std::clock_t endtime2 = std::clock();*/
	

	std::cout << "多线程版本耗时" << endtime1 - starttime << std::endl;

	//std::cout << "单线程版本耗时" << endtime2 - endtime1 << std::endl;

	return 1;
}

