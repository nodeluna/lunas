module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <algorithm>
#	include <ranges>
#	include <unordered_set>
#	include <functional>
#endif

export module lunas.filter:find;

export namespace lunas
{
	bool find(const std::string& path, const std::unordered_set<std::string>& files, const std::unordered_set<std::string>& pattern,
		  std::function<bool(void)> final_check);
}

namespace lunas
{
	bool find(const std::string& path, const std::unordered_set<std::string>& files, const std::unordered_set<std::string>& pattern,
		  std::function<bool(void)> final_check)
	{
		if (not files.empty())
		{
			auto itr = files.find(path);
			if (itr != files.end())
			{
				return true;
			}
			bool found =
			    std::ranges::any_of(files,
						[&](const std::string& x_path)
						{
							return path.size() > x_path.size() && path.substr(0, x_path.size()) == x_path;
						});
			if (found)
			{
				return true;
			}
		}

		if (not pattern.empty())
		{
			auto itr = pattern.find(path);
			if (itr != pattern.end())
			{
				return true;
			}
			bool found = std::ranges::any_of(pattern,
							 [&](const std::string& x_pattern)
							 {
								 return path.find(x_pattern) != path.npos;
							 });
			if (found)
			{
				return true;
			}
		}

		return final_check();
	}
}
