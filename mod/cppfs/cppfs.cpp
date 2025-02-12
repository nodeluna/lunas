module;

#include <filesystem>
#include <string>
#include <expected>
#include <system_error>
#include <variant>
#include <cerrno>
#include <cstring>

export module lunas.cppfs;
import lunas.error;

export namespace lunas {
	namespace cppfs {
		std::expected<std::monostate, lunas::error> remove(const std::string& path, bool dry_run);
		std::expected<std::monostate, lunas::error> mkdir(const std::string& path, bool dry_run);
		std::expected<std::monostate, lunas::error> symlink(const std::string& target, const std::string& dest, bool dry_run);
		std::expected<std::uintmax_t, lunas::error> file_size(const std::string& path);

	}
}

namespace lunas {
	namespace cppfs {
		std::expected<std::monostate, lunas::error> remove(const std::string& path, bool dry_run) {
			std::error_code ec;
			if (dry_run == false)
				std::filesystem::remove(path, ec);

			if (ec.value() != 0) {
				std::string err = "couldn't remove file '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::cppfs_remove));
			}
			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> mkdir(const std::string& path, bool dry_run) {
			std::error_code ec;
			if (dry_run == false)
				std::filesystem::create_directory(path, ec);

			if (ec.value() != 0) {
				std::string err = "couldn't mkdir '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::cppfs_mkdir));
			}
			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> symlink(const std::string& target, const std::string& dest, bool dry_run) {
			std::error_code ec;
			if (dry_run == false)
				std::filesystem::create_symlink(target, dest, ec);

			if (ec.value() != 0) {
				std::string err = "couldn't make symlink '" + dest + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::cppfs_symlink));
			}
			return std::monostate();
		}

		std::expected<std::uintmax_t, lunas::error> file_size(const std::string& path) {
			std::error_code ec;
			std::uintmax_t	size = std::filesystem::file_size(path, ec);

			if (ec.value() != 0) {
				std::string	  err = "couldn't get file size of '" + path + "', " + std::strerror(errno);
				lunas::error_type type =
				    errno == ENOENT ? lunas::error_type::attributes_no_such_file : lunas::error_type::cppfs_file_size;
				return std::unexpected(lunas::error(err, type));
			}
			return size;
		}
	}
}
