module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <filesystem>
#	include <string>
#	include <expected>
#	include <system_error>
#	include <variant>
#	include <cerrno>
#	include <cstring>
#endif

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
				std::string err = "couldn't remove file '" + path + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::cppfs_remove));
			}
			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> mkdir(const std::string& path, bool dry_run) {
			std::error_code ec;
			if (dry_run == false)
				std::filesystem::create_directory(path, ec);

			if (ec.value() != 0) {
				std::string err = "couldn't mkdir '" + path + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::cppfs_mkdir));
			}
			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> symlink(const std::string& target, const std::string& dest, bool dry_run) {
			std::error_code ec;
			if (dry_run == false)
				std::filesystem::create_symlink(target, dest, ec);

			if (ec.value() != 0) {
				std::string err = "couldn't make symlink '" + dest + "', " + ec.message();
				return std::unexpected(lunas::error(err, lunas::error_type::cppfs_symlink));
			}
			return std::monostate();
		}

		std::expected<std::uintmax_t, lunas::error> file_size(const std::string& path) {
			std::error_code ec;
			std::uintmax_t	size = std::filesystem::file_size(path, ec);

			if (ec.value() != 0) {
				std::string	  err  = "couldn't get file size of '" + path + "', " + ec.message();
				lunas::error_type type = ec == std::errc::no_such_file_or_directory
							     ? lunas::error_type::attributes_no_such_file
							     : lunas::error_type::cppfs_file_size;
				return std::unexpected(lunas::error(err, type));
			}
			return size;
		}
	}
}
