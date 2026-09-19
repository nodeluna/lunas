#include <cassert>

#include <string>
#include <unordered_set>

#include "allow.hpp"
#include "find.hpp"

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
