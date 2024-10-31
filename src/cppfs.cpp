#include <filesystem>
#include <string>
#include <expected>
#include "config.h"

namespace cppfs {
	std::error_code remove(const std::string& path) {
		std::error_code ec;
		if (options::dry_run == false)
			std::filesystem::remove(path, ec);
		return ec;
	}

	/*std::filesystem::file_status status(const std::string& path, std::error_code& ec){
		if(options::follow_symlink)
			return std::filesystem::status(path, ec);
		else
			return std::filesystem::symlink_status(path, ec);
	}*/
	void copy(const std::string& src, const std::string& dest, std::error_code& ec) {
		std::filesystem::copy_options opts = std::filesystem::copy_options::none;

		if (options::follow_symlink)
			opts = opts | std::filesystem::copy_options::copy_symlinks;

		if (options::update)
			opts = opts | std::filesystem::copy_options::update_existing;
		else if (options::rollback)
			opts = opts | std::filesystem::copy_options::overwrite_existing;

		if (options::dry_run == false) {
			std::filesystem::copy(src, dest + ".ls.part", opts, ec);
			if (ec.value() != 0)
				return;
			std::filesystem::rename(dest + ".ls.part", dest, ec);
		}
	}

	void mkdir(const std::string& dest, std::error_code& ec) {
		if (options::dry_run == false)
			std::filesystem::create_directory(dest, ec);
	}

	void symlink(const std::string& target, const std::string& dest, std::error_code& ec) {
		if (options::dry_run == false)
			std::filesystem::create_symlink(target, dest, ec);
	}

	std::expected<std::uintmax_t, std::error_code> file_size(const std::string& path) {
		std::error_code ec;
		std::uintmax_t	size = std::filesystem::file_size(path, ec);
		if (ec.value() != 0)
			return std::unexpected(ec);
		return size;
	}
}
