#include <string>

#include "path.hpp"

namespace path
{
	void append_seperator(std::string& path) noexcept
	{
		if (path.empty() != true && path.back() != path_seperator)
		{
			path = path + path_seperator;
		}
	}

	void pop_seperator(std::string& path) noexcept
	{
		if (path.empty() != true && path.back() == path_seperator)
		{
			path.pop_back();
		}
	}
}
