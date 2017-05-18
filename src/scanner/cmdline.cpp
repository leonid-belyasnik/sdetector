#include "cmdline.h"
#include <algorithm>

namespace CmdLine {

	char* get_option(char ** begin, char ** end, const std::string & option, const std::string & s_option)
	{
		char ** itr = find(begin, end, option);
		if (itr == end)
			itr = find(begin, end, s_option);

		if (itr != end && ++itr != end)
		{
			if ((*itr)[0] == '-')
				return 0;

			return *itr;
		}
		return 0;
	}

	bool option_exists(char** begin, char** end, const std::string& option, const std::string & s_option)
	{
		if (find(begin, end, option) == end)
			return find(begin, end, s_option) != end;

		return true;
	}
}