#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <algorithm>
#	include <ranges>
#	include <unordered_set>
#	include <functional>
#endif

namespace lunas
{
	bool find(const std::string& path, const std::unordered_set<std::string>& files, const std::unordered_set<std::string>& pattern,
		  std::function<bool(void)> final_check);
}
