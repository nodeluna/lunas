#pragma once

#include <string>
#include <unordered_set>
#include <functional>

namespace lunas
{
	bool find(const std::string& path, const std::unordered_set<std::string>& files, const std::unordered_set<std::string>& pattern,
		  std::function<bool(void)> final_check);
}
