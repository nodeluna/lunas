#ifndef CPPFS
#define CPPFS

#include <filesystem>
#include <string>

namespace cppfs{
	/*
	std::error_code remove(const std::string& path);
	std::filesystem::file_status status(const std::string& path, std::error_code& ec);
	*/
	std::filesystem::file_status copy(const std::string& src, const std::string& dest, std::error_code& ec);
	void mkdir(const std::string& dest, std::error_code& ec);
}

#endif // CPPFS
