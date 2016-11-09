#ifndef WIND_DATA_REPAIR_H
#define WIND_DATA_REPAIR_H

#include <vector>
#include <map>
#include <set>
#include <queue>
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
#include "timeutil.h"
/************使用前日期数据必须先排序**************/
namespace JGG
{
	struct WindDataOnTime
	{
		JGG::TimePoint currTime;

		std::vector<std::string> restdata;

		bool operator < (const WindDataOnTime& winddataontime) const
		{
			return this->currTime < winddataontime.currTime;
		}
	};


	class WindDataRepair
	{
		using QueueWindData = std::set<WindDataOnTime>;
	public:
		WindDataRepair();
		~WindDataRepair();

		// csv文件格式
		void Start(const std::string& incsvFile,const std::string& outcsvFile);
	private:
		std::queue<QueueWindData> m_filereadtoanalysis;
		std::queue<QueueWindData> m_analysistofileout;
		std::vector<std::string> m_fileheader;

		void ReadFileDataToQueue(const std::string& datafile);
		void AnalysisDataToQueue(const QueueWindData& curdata);
		void OutQueueToFile(std::ofstream* ofs, const QueueWindData& curdata);
	};
}

#endif



