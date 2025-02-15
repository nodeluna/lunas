module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <algorithm>
#	include <ranges>
#	include <unordered_set>
#endif

export module lunas.exclude;

export namespace lunas {
	bool exclude(const std::string& path, const std::unordered_set<std::string>& exclude_files,
	    const std::unordered_set<std::string>& exclude_pattern);
}

namespace lunas {
	bool exclude(const std::string& path, const std::unordered_set<std::string>& exclude_files,
	    const std::unordered_set<std::string>& exclude_pattern) {

		if (exclude_files.empty() && exclude_pattern.empty())
			return false;

		if (not exclude_files.empty()) {
			auto itr = exclude_files.find(path);
			if (itr != exclude_files.end())
				return true;
			bool excluded = std::ranges::any_of(exclude_files, [&](const std::string& x_path) {
				return path.size() > x_path.size() && path.substr(0, x_path.size()) == x_path;
			});
			if (excluded)
				return true;
		}

		if (not exclude_pattern.empty()) {
			auto itr = exclude_pattern.find(path);
			if (itr != exclude_pattern.end())
				return true;
			bool excluded = std::ranges::any_of(
			    exclude_pattern, [&](const std::string& x_pattern) { return path.find(x_pattern) != path.npos; });
			if (excluded)
				return true;
		}

		return false;
	}
}
