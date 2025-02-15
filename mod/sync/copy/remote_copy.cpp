module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <memory>
#	include <string>
#	include <expected>
#endif

export module lunas.sync:remote_copy;
export import :types;
export import :stdout;
export import :local_to_remote;
export import :remote_to_local;
export import :remote_to_remote;
export import :remote_attributes;

export import lunas.sftp;
export import lunas.error;
export import lunas.stdout;

#ifdef REMOTE_ENABLED

export namespace lunas {
	namespace remote {
		std::expected<lunas::syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
		    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp,
		    const lunas::syncmisc& misc) {

			lunas::print_sync(src, dest, misc);

			std::expected<struct syncstat, lunas::error> syncstat;

			if (src_sftp != nullptr && dest_sftp == nullptr)
				syncstat = lunas::remote_to_local::copy(src, dest, src_sftp, misc);
			else if (src_sftp == nullptr && dest_sftp != nullptr)
				syncstat = lunas::local_to_remote::copy(src, dest, dest_sftp, misc);
			else
				syncstat = lunas::remote_to_remote::copy(src, dest, src_sftp, dest_sftp, misc);

			if (not syncstat)
				return std::unexpected(syncstat.error());

			if (misc.options.dry_run == false && syncstat.value().code == lunas::sync_code::success) {
				auto ok = lunas::utimes::remote(src, dest, src_sftp, dest_sftp, misc);
				if (not ok) {
					lunas::warn("{}", ok.error().message());
				}
			}

			return syncstat;
		}
	}
}

#endif // REMOTE_ENABLED
