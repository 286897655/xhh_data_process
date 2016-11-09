#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <sstream>
#include <vector>

namespace JGG{
	std::vector<std::string> splitstring(const std::string& srcstring, char sep = ',');

	template<typename T>
	std::string itostr(const T& t)
		// Converts integer T to string.
	{
		std::ostringstream oss;
		oss << t;
		return oss.str();
	}

	template<typename T>
	T strtoi(const std::string& s)
		// Converts string to integer T.
		// Ensure the integer type has  
		// sufficient storage capacity.
	{
		std::istringstream iss(s);
		T x;
		if (!(iss >> x))
			return 0;
		return x;
	}
}
#endif