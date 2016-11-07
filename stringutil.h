#ifndef STRING_UTIL_H
#define STRING_UTIL_H

#include <string>
#include <sstream>

namespace stringutil{


	std::vector<std::string> splitstring(const std::string& srcstring, char sep = ',')
	{
		std::vector<std::string> vecsep;
		if (srcstring.empty())
			return vecsep;

		std::string::size_type pos_begin = srcstring.find_first_not_of(sep);
		std::string::size_type comma_pos = 0;

		std::string tmp;
		while (pos_begin != std::string::npos)
		{
			comma_pos = srcstring.find(sep, pos_begin);
			if (comma_pos != std::string::npos)
			{
				tmp = srcstring.substr(pos_begin, comma_pos - pos_begin);
				pos_begin = comma_pos + 1;
			}
			else
			{
				tmp = srcstring.substr(pos_begin);
				pos_begin = comma_pos;
			}

			if (!tmp.empty())
			{
				vecsep.push_back(tmp);
				tmp.clear();
			}
		}

		return vecsep;
	}

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