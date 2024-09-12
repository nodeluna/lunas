#ifndef CPPFS
#define CPPFS

#include <filesystem>
#include <string>

namespace cppfs{
	std::error_code remove(const std::string& path);
	//std::filesystem::file_status status(const std::string& path, std::error_code& ec);
	std::filesystem::file_status copy(const std::string& src, const std::string& dest, std::error_code& ec);
	void mkdir(const std::string& dest, std::error_code& ec);
	void symlink(const std::string& target, const std::string& dest, std::error_code& ec);
	std::expected<std::uintmax_t, std::error_code> file_size(const std::string& path);
}

#endif // CPPFS
