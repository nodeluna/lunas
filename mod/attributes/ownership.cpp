module;

#include <string>
#include <expected>
#include <variant>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

export module lunas.attributes:ownership;
export import lunas.error;
export import lunas.file_types;

export namespace lunas {
	namespace ownership {
		struct own {
				int uid = -1;
				int gid = -1;
		};

		std::expected<struct own, lunas::error>	    get(const std::string& path, lunas::follow_symlink follow);
		std::expected<std::monostate, lunas::error> set(
		    const std::string& path, const struct own& own, lunas::follow_symlink follow);
	}
}

namespace lunas {
	namespace ownership {
		std::expected<struct own, lunas::error> get(const std::string& path, lunas::follow_symlink follow) {
			struct own  own;
			struct stat stats;
			int	    rc = 0;
			if (follow == lunas::follow_symlink::yes)
				rc = stat(path.c_str(), &stats);
			else
				rc = lstat(path.c_str(), &stats);
			if (rc != 0) {
				std::string err = "couldn't get ownership of '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::attributes_get_ownership));
			}

			own.uid = stats.st_uid;
			own.gid = stats.st_gid;
			return own;
		}

		std::expected<std::monostate, lunas::error> set(
		    const std::string& path, const struct own& own, lunas::follow_symlink follow) {
			int rc = 0;

			if (follow == lunas::follow_symlink::yes)
				rc = chown(path.c_str(), own.uid, own.gid);
			else
				rc = lchown(path.c_str(), own.uid, own.gid);

			if (rc != 0) {
				std::string err = "couldn't set ownership of '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::attributes_set_ownership));
			}

			return std::monostate();
		}
	}
}
