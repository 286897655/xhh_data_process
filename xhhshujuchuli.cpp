// xhhshujuchuli.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <algorithm>
#include <queue>
#include "timeutil.h"
#include "stringutil.h"

struct WindTime
{
	timeutil::TimePoint currTime;

	std::vector<std::string> restdata;
};

static std::vector<std::string> Head;

static std::queue<>


void OutDataToCSV(const std::string& csvfile, const std::map<std::string, std::vector<WindTime>>& AllData)
{
	// ������csv�ļ�
	std::ofstream ofs(csvfile, std::ios_base::out);

	if (!ofs.is_open())
	{
		printf_s("���ܴ��ļ�\n");
		throw std::runtime_error("can't open file");
	}
	printf_s("��ʼ�������\n");

	// ���ͷ
	for each(const auto& var in Head)
	{
		ofs << var << ",";
	}
	ofs << '\n';

	for each(const auto& var in AllData)
	{
		printf_s("��ǰ�������:%s\n", var.first.c_str());
		
		for  each(const auto& windtime in var.second)
		{
			std::tm* nowtime = timeutil::TimePointTotm(windtime.currTime);
			
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

	printf_s("����ļ�����\n");
}

std::map<std::string, std::vector<WindTime>> ReadDataByCSV(const std::string& csvfile)
{
	// ������csv�ļ�
	std::ifstream ifs(csvfile, std::ios_base::in);
	if (!ifs.is_open())
	{
		printf_s("���ܴ��ļ�\n");
		throw std::runtime_error("open file error");
	}
	printf_s("��ʼ��ȡ����\n");
	std::map<std::string, std::vector<WindTime>> allDataByDate;
	int i = 0;
	// read all in memery
	std::stringstream buffer;
	buffer << ifs.rdbuf();
	ifs.close();
	while (!buffer.eof())
	{
		i++;
		printf_s("��ʼ��ȡ��:------%d-----��\n", i);
		std::string readstr;
		buffer >> readstr;

		std::vector<std::string> splited = stringutil::splitstring(readstr);

		if (splited.empty())
			continue;
		if (1 == i)
		{
			Head = splited;
			continue;
		}	

		WindTime tmpdata;
		tmpdata.currTime = timeutil::StringToT(splited[0] + "+" + splited[1], "%d/%d/%d+%d:%d:%d");

		for (std::vector<std::string>::const_iterator it = splited.begin() + 2; it != splited.end(); it++)
		{
			tmpdata.restdata.push_back(*it);
		}

		allDataByDate[splited[0]].push_back(tmpdata);

	}
	printf_s("��ȡ�ļ�����\n");
	return allDataByDate;
}


int main(int argc, char* argv[])
{
	std::clock_t time_start = std::clock();
	std::map<std::string, std::vector<WindTime>> allDataByDate = ReadDataByCSV("winddata.csv");
	std::clock_t time_end = std::clock();

	printf_s("��ȡ�ļ���ʱ:%d\n", time_end - time_start);

	const std::string startDate= allDataByDate.begin()->first;
	const std::string endDate = allDataByDate.rbegin()->first;
	
	std::map<std::string, std::vector<WindTime>> NewAllData;

	for each (const auto& var in allDataByDate)
	{
		printf_s("�ļ����ݿ�ʼ����:%s---�ļ����ݽ�������:%s---\n", startDate.c_str(), endDate.c_str());
		printf_s("��ǰ������������:%s\n", var.first.c_str());
		printf_s("��ǰ������������:%d\n", var.second.size());

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

		// ��ͷ
		while (StartTime_Point < dayData[0].currTime)
		{
			WindTime newtimedata;
			newtimedata.currTime = StartTime_Point;

			newtimedata.restdata = std::vector<std::string>(Head.size()-2, "9999");//-2ȥ�����ں�ʱ��

			StartTime_Point += std::chrono::microseconds(10000000);

			newData.push_back(newtimedata);
		}
		// ����
		for each (const auto& windtime in dayData)
		{
			// ����ʼʱ��С������ֱ�ӱ���
			auto period1 = windtime.currTime - StartTime_Point;
			if (period1.count() <= 0)
			{
				newData.push_back(windtime);
				continue;
			}
			// ����ֹʱ��������ֱ�ӱ���
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

					newtimedata.restdata = std::vector<std::string>(Head.size() - 2, "9999");//-2ȥ�����ں�ʱ��

					newData.push_back(newtimedata);
				}
			}
		}

		// ��β
		while (StartTime_Point < EndTime_Point)
		{
			StartTime_Point += std::chrono::microseconds(10000000);
			WindTime newtimedata;
			newtimedata.currTime = StartTime_Point;

			newtimedata.restdata = std::vector<std::string>(Head.size() - 2, "9999");//-2ȥ�����ں�ʱ��

			newData.push_back(newtimedata);
		}

		printf_s("����%s����,������Ϊ:%d\n", var.first.c_str(), newData.size());

		NewAllData[var.first] = newData;
	}

	time_start = std::clock();

	std::cout << "���ݷ�����ʱ" << time_start-time_end << std::endl;

	// ����ļ���csv�ļ�
	std::cout << "���ݲ������---" << "������ݵ�csv�ļ�" << std::endl;

	OutDataToCSV(std::string("OutPutData.csv"), NewAllData);

	time_end = std::clock();
	std::cout << "����������ļ���ʱ" << time_end - time_start << std::endl;
	return 0;
}

