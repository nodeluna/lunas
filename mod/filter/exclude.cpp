#include <string>
#include <unordered_set>

#include "exclude.hpp"
#include "find.hpp"

namespace lunas
{
	bool exclude(const std::string& path, const std::unordered_set<std::string>& exclude_files,
		     const std::unordered_set<std::string>& exclude_pattern)
	{

		auto final_check = [&]()
		{
			return false;
		};

		return lunas::find(path, exclude_files, exclude_pattern, final_check);
	}
}
