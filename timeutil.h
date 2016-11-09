#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <string>
#include <ctime>
#include <stdint.h>
#include <chrono>

namespace JGG{
	using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

	TimePoint StringToT(const std::string& str, const char* fmt);

	std::tm* TimePointTotm(const TimePoint& timepoint);

	static const char* ISO8601Format = "%Y-%m-%dT%H:%M:%SZ";
	// The date/time format defined in the ISO 8601 standard.
	// This is the default format used throughout the library for consistency.
	//
	// Examples:
	//   2005-01-01T12:00:00+01:00
	//   2005-01-01T11:00:00Z

	std::time_t now();
	// Return the number of UTC milliseconds since epoch.

	double clockSecs();
	// Return the current process time in decimal seconds.

	std::string print(const std::tm& dt, const char* fmt = ISO8601Format);
	// Cross-platform time formatting.

	std::string printLocal(const char* fmt = ISO8601Format);
	// Print the current local time using the given format.

	std::string printUTC(const char* fmt = ISO8601Format);
	// Print the current UTC time using the given format.

	std::tm toLocal(const std::time_t& time);
	// Convert the given time value to local time.
	// Uses thread-safe native functions.

	std::tm toUTC(const std::time_t& time);
	// Convert the given time value to UTC time.
	// Uses thread-safe native functions.

	std::string getLocal();
	// Return a local ISO8601 formatted date time string.

	std::string getUTC();
	// Return a UTC ISO8601 formatted date time string.
}

#endif