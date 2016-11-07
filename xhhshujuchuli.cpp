// xhhshujuchuli.cpp : 定义控制台应用程序的入口点。
//

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include "timeutil.h"
#include "stringutil.h"

struct WindTime
{
	timeutil::TimePoint currTime;

	float WIND_SPEED;
	float WIND_DIR;
	float OUTPUT_POWER;
	int STATE;
};

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

	ofs << "RECDATE" << "," << "RECTIME" << ","
		<< "WINDSPEED" << "," << "WINDIR" << "," << "OUTPUTPOWER"
		<< "," << "STATE" << '\n';

	for each(const auto& var in AllData)
	{
		printf_s("当前输出日期:%s\n", var.first.c_str());
		
		for  each(const auto& windtime in var.second)
		{
			std::tm* nowtime = timeutil::TimePointTotm(windtime.currTime);
			
			char TimeFormat[128];
			sprintf_s(TimeFormat, "%d:%d:%d", nowtime->tm_hour, nowtime->tm_min, nowtime->tm_sec);

			std::string NewTime(TimeFormat);

			ofs << var.first << "," << NewTime << "," << windtime.WIND_SPEED
				<< "," << windtime.WIND_DIR << "," << windtime.OUTPUT_POWER << ","
				<< windtime.STATE << '\n';
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
		i++;
		printf_s("开始读取第:------%d-----行\n", i);
		std::string readstr;
		buffer >> readstr;

		std::vector<std::string> splited = stringutil::splitstring(readstr);

		if (splited.empty() || 1 == i)
			continue;

		WindTime tmpdata;
		tmpdata.currTime = timeutil::StringToT(splited[0] + "+" + splited[1], "%d/%d/%d+%d:%d:%d");
		tmpdata.WIND_SPEED = atof(splited[2].c_str());//stringutil::strtoi<float>(splited[2]);//
		tmpdata.WIND_DIR = atof(splited[3].c_str());//stringutil::strtoi<float>(splited[3]);//
		tmpdata.OUTPUT_POWER = atof(splited[4].c_str());//stringutil::strtoi<float>(splited[4]);//
		tmpdata.STATE = atoi(splited[5].c_str());//stringutil::strtoi<int>(splited[5]);//

		allDataByDate[splited[0]].push_back(tmpdata);

	}
	printf_s("读取文件结束\n");
	return allDataByDate;
}


int main(int argc, char* argv[])
{
	std::clock_t time_start = std::clock();
	std::map<std::string, std::vector<WindTime>> allDataByDate = ReadDataByCSV("winddata.csv");
	std::clock_t time_end = std::clock();

	printf_s("读取文件耗时:%d\n", time_end - time_start);

	const std::string startDate= allDataByDate.begin()->first;
	const std::string endDate = allDataByDate.rbegin()->first;
	
	std::map<std::string, std::vector<WindTime>> NewAllData;

	for each (const auto& var in allDataByDate)
	{
		printf_s("文件数据开始日期:%s---文件数据结束日期:%s---\n", startDate.c_str(), endDate.c_str());
		printf_s("当前处理数据日期:%s\n", var.first.c_str());
		printf_s("当前日期数据总数:%d\n", var.second.size());

		std::vector<WindTime> dayData = var.second;

		std::sort(dayData.begin(), dayData.end(), [](const WindTime& first,const WindTime& second)->int{
			return (first.currTime <= second.currTime) ? 1 : 0;
		});

		// Start Time 
		const std::string starttime = "00:50:10";
		// End Time
		const std::string endtime = "23:50:00";

		timeutil::TimePoint StartTime_Point = timeutil::StringToT(var.first + "+" + starttime, "%d/%d/%d+%d:%d:%d");

		timeutil::TimePoint EndTime_Point = timeutil::StringToT(var.first + "+" + endtime, "%d/%d/%d+%d:%d:%d");

		std::vector<WindTime> newData;

		// 补头
		while (StartTime_Point < dayData[0].currTime)
		{
			WindTime newtimedata;
			newtimedata.currTime = StartTime_Point;
			newtimedata.OUTPUT_POWER = 9999;
			newtimedata.STATE = 9999;
			newtimedata.WIND_DIR = 9999;
			newtimedata.WIND_SPEED = 9999;

			StartTime_Point += std::chrono::microseconds(10000000);

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
				newData.push_back(windtime);
			}

			while (true)
			{
				StartTime_Point += std::chrono::microseconds(10000000);
				if (StartTime_Point == windtime.currTime)
				{
					newData.push_back(windtime);
					break;
				}
				else
				{
					WindTime newtimedata;
					newtimedata.currTime = StartTime_Point;
					newtimedata.OUTPUT_POWER = 9999;
					newtimedata.STATE = 9999;
					newtimedata.WIND_DIR = 9999;
					newtimedata.WIND_SPEED = 9999;

					newData.push_back(newtimedata);
				}
			}
		}

		// 补尾
		while (StartTime_Point < EndTime_Point)
		{
			StartTime_Point += std::chrono::microseconds(10000000);
			WindTime newtimedata;
			newtimedata.currTime = StartTime_Point;
			newtimedata.OUTPUT_POWER = 9999;
			newtimedata.STATE = 9999;
			newtimedata.WIND_DIR = 9999;
			newtimedata.WIND_SPEED = 9999;

			newData.push_back(newtimedata);
		}

		printf_s("补完%s数据,总数据为:%d\n", var.first.c_str(), newData.size());

		NewAllData[var.first] = newData;
	}

	time_start = std::clock();

	std::cout << "数据分析耗时" << time_start-time_end << std::endl;

	// 输出文件到csv文件
	std::cout << "数据补充结束---" << "输出数据到csv文件" << std::endl;

	OutDataToCSV(std::string("OutPutData.csv"), NewAllData);

	time_end = std::clock();
	std::cout << "数据输出到文件耗时" << time_end - time_start << std::endl;
	return 0;
}

