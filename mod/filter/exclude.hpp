#pragma once

#include <string>
#include <unordered_set>

namespace lunas
{
	bool exclude(const std::string& path, const std::unordered_set<std::string>& exclude_files,
		     const std::unordered_set<std::string>& exclude_pattern);
}
