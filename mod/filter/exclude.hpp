#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <algorithm>
#	include <ranges>
#	include <unordered_set>
#endif

namespace lunas
{
	bool exclude(const std::string& path, const std::unordered_set<std::string>& exclude_files,
		     const std::unordered_set<std::string>& exclude_pattern);
}
