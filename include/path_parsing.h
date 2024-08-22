#ifndef PATH_PARSING
#define PATH_PARSING


namespace parse_path{
	void adjust_relative_path(std::string& path, int& depth);

	void append_to_relative_path(std::string& path, std::string& current_path, int& depth);

	std::string absolute(const std::string& path);
}

#endif // PATH_PARSING