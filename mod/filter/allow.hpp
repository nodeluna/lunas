#pragma once

#include <string>
#include <unordered_set>

namespace lunas
{
	bool allow(const std::string& path, const std::unordered_set<std::string>& allow_files,
		   const std::unordered_set<std::string>& allow_pattern);
}
