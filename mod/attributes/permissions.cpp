module;

#include <filesystem>
#include <expected>
#include <variant>
#include <system_error>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h>

export module lunas.attributes:permissions;
export import lunas.file_types;
export import lunas.error;

namespace fs = std::filesystem;

export namespace lunas {
	namespace permissions {
		std::expected<bool, lunas::error> is_file_readable(const std::string& path, lunas::follow_symlink follow);

		std::expected<std::filesystem::perms, lunas::error> get(const std::string& path, lunas::follow_symlink follow);

		std::expected<std::monostate, lunas::error> set(
		    const std::string& path, std::filesystem::perms permissions, lunas::follow_symlink follow);

		std::expected<std::monostate, lunas::error> set(
		    const std::string& path, unsigned int permissions, lunas::follow_symlink follow);
	}
}

namespace lunas {
	namespace permissions {
		std::expected<bool, lunas::error> is_file_readable(const std::string& path, lunas::follow_symlink follow) {
			auto perms = permissions::get(path, follow);
			if (not perms) {
				std::string err = "couldn't check permission '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::attributes_permissions_check));
			}

			if ((perms.value() & fs::perms::owner_read) != fs::perms::none)
				return true;
			return false;
		}

		std::expected<std::filesystem::perms, lunas::error> get(const std::string& path, lunas::follow_symlink follow) {
			std::error_code	       ec;
			std::filesystem::perms perms;
			if (follow == lunas::follow_symlink::yes)
				perms = std::filesystem::status(path, ec).permissions();
			else
				perms = std::filesystem::symlink_status(path, ec).permissions();
			if (ec.value() != 0) {
				std::string err = "couldn't get permission '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::attributes_set_permissions));
			}
			return perms;
		}

		std::expected<std::monostate, lunas::error> set(
		    const std::string& path, std::filesystem::perms permissions, lunas::follow_symlink follow) {
			std::error_code		      ec;
			std::filesystem::perm_options perm_options = std::filesystem::perm_options::replace;

			if (follow == lunas::follow_symlink::no)
				perm_options |= std::filesystem::perm_options::nofollow;

			std::filesystem::permissions(path, permissions, perm_options, ec);
			if (ec.value() != 0) {
				std::string err = "couldn't set permission '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::attributes_set_permissions));
			}
			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> set(
		    const std::string& path, unsigned int permissions, lunas::follow_symlink follow) {
			return set(path, ( std::filesystem::perms ) permissions, follow);
		}
	}
}
