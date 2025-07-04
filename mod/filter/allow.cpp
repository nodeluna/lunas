module;

#include <cassert>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <algorithm>
#	include <ranges>
#	include <unordered_set>
#endif

export module lunas.filter:allow;
import :find;

export namespace lunas
{
	bool allow(const std::string& path, const std::unordered_set<std::string>& allow_files,
		   const std::unordered_set<std::string>& allow_pattern);
}

namespace lunas
{
	bool allow(const std::string& path, const std::unordered_set<std::string>& allow_files,
		   const std::unordered_set<std::string>& allow_pattern)
	{
		auto final_check = [&]()
		{
			return (allow_files.empty() && allow_pattern.empty()) ? true : false;
		};

		return lunas::find(path, allow_files, allow_pattern, final_check);
	}
}
