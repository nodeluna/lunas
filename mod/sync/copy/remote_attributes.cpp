module;

#include <string>
#include <memory>
#include <expected>
#include <variant>

export module lunas.sync:remote_attributes;
export import :types;
export import lunas.error;
export import lunas.sftp;
export import lunas.attributes;

export namespace lunas {
	namespace ownership {
		std::expected<std::monostate, lunas::error> local_to_remote(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc);

		std::expected<std::monostate, lunas::error> remote_to_local(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc);

		std::expected<std::monostate, lunas::error> remote_to_remote(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp, const syncmisc& misc);
	}

	namespace permissions {
		std::expected<std::monostate, lunas::error> remote_to_local(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc);
	}

	namespace utimes {
		std::expected<std::monostate, lunas::error> remote(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp, const syncmisc& misc);

	}
}

namespace lunas {
	namespace ownership {
		std::expected<std::monostate, lunas::error> local_to_remote(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc) {
			if (misc.options.attributes_uid || misc.options.attributes_gid) {
				auto own = lunas::ownership::get(src, misc.options.follow_symlink);
				if (not own)
					return std::unexpected(own.error());

				struct lunas::sftp::owner owner = {
				    .uid = misc.options.attributes_uid ? own.value().uid : -1,
				    .gid = misc.options.attributes_gid ? own.value().gid : -1,
				};

				auto ok2 = sftp->set_ownership(dest, owner, misc.options.follow_symlink);
				if (not ok2)
					return std::unexpected(ok2.error());
			}

			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> remote_to_local(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc) {
			if (misc.options.attributes_uid || misc.options.attributes_gid) {
				auto own = sftp->get_ownership(src, misc.options.follow_symlink);
				if (not own)
					return std::unexpected(own.error());

				struct lunas::ownership::own owner = {
				    .uid = misc.options.attributes_uid ? own.value().uid : -1,
				    .gid = misc.options.attributes_gid ? own.value().gid : -1,
				};

				auto ok = lunas::ownership::set(dest, owner, misc.options.follow_symlink);
				if (not ok)
					return std::unexpected(ok.error());
			}

			return std::monostate();
		}

		std::expected<std::monostate, lunas::error> remote_to_remote(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp, const syncmisc& misc) {
			if (not misc.options.attributes_uid && not misc.options.attributes_gid)
				return std::monostate();

			auto own = src_sftp->get_ownership(src, misc.options.follow_symlink);
			if (not own)
				return std::unexpected(own.error());

			auto ok = dest_sftp->set_ownership(dest, own.value(), misc.options.follow_symlink);
			if (not ok) {
				return std::unexpected(ok.error());
			}

			return std::monostate();
		}
	}

	namespace permissions {
		std::expected<std::monostate, lunas::error> remote_to_local(
		    const std::string& src, const std::string& dest, const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc) {
			auto perms = sftp->get_permissions(src, misc.options.follow_symlink);
			if (not perms)
				return std::unexpected(perms.error());

			auto ok = lunas::permissions::set(dest, perms.value(), misc.options.follow_symlink);
			if (not ok)
				return std::unexpected(ok.error());

			return std::monostate();
		}
	}

	namespace utimes {
		std::expected<std::monostate, lunas::error> remote(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp, const syncmisc& misc) {
			if (not misc.options.attributes_atime && not misc.options.attributes_mtime)
				return std::monostate();

			lunas::time_type utime;

			if (misc.options.attributes_atime && not misc.options.attributes_mtime)
				utime = lunas::time_type::atime;
			else if (not misc.options.attributes_atime && misc.options.attributes_mtime)
				utime = lunas::time_type::mtime;
			else
				utime = lunas::time_type::utimes;

			std::expected<struct lunas::time_val, lunas::error> time_val;

			if (src_sftp != nullptr) {
				time_val = src_sftp->get_utimes<lunas::time_val>(src, utime, misc.options.follow_symlink);
				if (not time_val)
					return std::unexpected(time_val.error());
			} else {
				time_val = lunas::utime::get(src, utime, misc.options.follow_symlink);
				if (not time_val)
					return std::unexpected(time_val.error());
			}

			if (dest_sftp != nullptr) {
				auto ok = dest_sftp->set_utimes<lunas::time_val>(dest, time_val.value(), misc.options.follow_symlink);
				if (not ok)
					return std::unexpected(ok.error());
			} else {
				auto ok = lunas::utime::set(dest, time_val.value(), misc.options.follow_symlink);
				if (not ok)
					return std::unexpected(ok.error());
			}

			return std::monostate();
		}
	}
}
