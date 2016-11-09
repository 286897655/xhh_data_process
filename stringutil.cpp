#include "stringutil.h"

namespace JGG
{
	std::vector<std::string> splitstring(const std::string& srcstring, char sep)
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
}