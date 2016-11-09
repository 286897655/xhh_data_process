#include "timeutil.h"
#include <sstream>
#include <iomanip>

namespace JGG
{

	TimePoint StringToT(const std::string& str, const char* fmt)
	{
		int a[6] = { 0, 0, 0, 0, 0, 0 };
		sscanf_s(str.c_str(), fmt, a, a+1, a+2, a+3, a+4, a+5);
		
		std::tm tm_;
		tm_.tm_year = *a-1900;
		tm_.tm_mon = *(a+1);
		tm_.tm_mday = *(a + 2);
		tm_.tm_hour = *(a + 3);
		tm_.tm_min = *(a + 4);
		tm_.tm_sec = *(a + 5);
		tm_.tm_isdst = 0;	
		
		
		std::time_t time_t_tm = std::mktime(&tm_);

		return std::chrono::system_clock::from_time_t(time_t_tm);
	}

	std::tm* TimePointTotm(const TimePoint& timepoint)
	{
		std::time_t time_t_tm = std::chrono::system_clock::to_time_t(timepoint);

		return std::localtime(&time_t_tm);
	}

	std::time_t now()
	{
		return std::time(0);
	}

	double clockSecs()
	{
		return clock() / CLOCKS_PER_SEC;
	}


	std::tm toLocal(const std::time_t& time)
	{
		std::tm tm_snapshot;
#if defined(WIN32)
		localtime_s(&tm_snapshot, &time); // thread-safe?
#else
		localtime_r(&time, &tm_snapshot); // POSIX
#endif
		return tm_snapshot;
	}


	std::tm toUTC(const std::time_t& time)
	{
		// TODO: double check thread safety of native methods
		std::tm tm_snapshot;
#if defined(WIN32)
		gmtime_s(&tm_snapshot, &time); // thread-safe?
#else
		gmtime_r(&time, &tm_snapshot); // POSIX
#endif
		return tm_snapshot;
	}


	std::string print(const std::tm& dt, const char* fmt)
	{
#if defined(WIN32)
		// BOGUS hack done for VS2012: C++11 non-conformant since it SHOULD take a "const struct tm* "
		// ref. C++11 standard: ISO/IEC 14882:2011, � 27.7.1,
		std::ostringstream oss;
		oss << std::put_time(const_cast<std::tm*>(&dt), fmt);
		return oss.str();

#else    // LINUX
		const std::size_t size = 1024;
		char buffer[size];
		auto success = std::strftime(buffer, size, fmt, &dt);

		if (0 == success)
			return fmt;

		return buffer;
#endif
	}


	std::string printLocal(const char* fmt)
	{
		return print(toLocal(now()), fmt);
	}


	std::string printUTC(const char* fmt)
	{
		return print(toUTC(now()), fmt);
	}


	std::string getLocal()
	{
		return printLocal(ISO8601Format);
	}


	std::string getUTC()
	{
		return printUTC(ISO8601Format);
	}
}