module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <expected>
#	include <variant>
#endif

export module lunas.sync:local_attributes;
export import :types;
export import lunas.file_types;
export import lunas.error;
export import lunas.attributes;

export namespace lunas {
	namespace ownership {
		std::expected<std::monostate, lunas::error> local_to_local(
		    const std::string& src, const std::string& dest, const syncmisc& misc);
	}

	namespace utimes {
		std::expected<std::monostate, lunas::error> local(const std::string& src, const std::string& dest, const syncmisc& misc);
	}

	namespace permissions {
		std::expected<std::monostate, lunas::error> local_to_local(
		    const std::string& src, const std::string& dest, const syncmisc& misc);
	}
}

namespace lunas {
	namespace ownership {
		std::expected<std::monostate, lunas::error> local_to_local(
		    const std::string& src, const std::string& dest, const syncmisc& misc) {
			if (not misc.options.attributes_uid && not misc.options.attributes_gid)
				return std::monostate();

			auto own = lunas::ownership::get(src, misc.options.follow_symlink);
			if (not own)
				return std::unexpected(own.error());

			auto ok = lunas::ownership::set(dest, own.value(), misc.options.follow_symlink);
			if (not ok)
				return std::unexpected(ok.error());

			return std::monostate();
		}
	}

	namespace utimes {
		std::expected<std::monostate, lunas::error> local(const std::string& src, const std::string& dest, const syncmisc& misc) {
			if (not misc.options.attributes_atime && not misc.options.attributes_mtime)
				return std::monostate();

			lunas::time_type utime;

			if (misc.options.attributes_atime && not misc.options.attributes_mtime)
				utime = lunas::time_type::atime;
			else if (not misc.options.attributes_atime && misc.options.attributes_mtime)
				utime = lunas::time_type::mtime;
			else
				utime = lunas::time_type::utimes;

			auto utimes_return = lunas::utime::get(src, utime, misc.options.follow_symlink);
			if (not utimes_return)
				return std::unexpected(utimes_return.error());

			auto ok = lunas::utime::set(dest, utimes_return.value(), misc.options.follow_symlink);
			if (not ok)
				return std::unexpected(ok.error());

			return std::monostate();
		}
	}

	namespace permissions {
		std::expected<std::monostate, lunas::error> local_to_local(
		    const std::string& src, const std::string& dest, const syncmisc& misc) {
			auto perms = lunas::permissions::get(src, misc.options.follow_symlink);
			if (not perms)
				return std::unexpected(perms.error());

			auto ok = lunas::permissions::set(dest, perms.value(), misc.options.follow_symlink);
			if (not ok)
				return std::unexpected(ok.error());

			return std::monostate();
		}
	}
}
