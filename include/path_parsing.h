#ifndef PATH_PARSING
#define PATH_PARSING

#include <string>

namespace parse_path{
	std::string get_lower_dir_level(std::string path);

	void adjust_relative_path(std::string& path, int& depth);

	void append_to_relative_path(std::string& path, std::string& current_path, int& depth);

	std::string absolute(const std::string& path);
}

#endif // PATH_PARSING
