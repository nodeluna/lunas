module;

#include <expected>
#include <string>
#include <memory>

export module lunas.sync:copy;
export import :types;
export import :remote_copy;
export import :local_copy;
export import lunas.sftp;

import lunas.error;

export namespace lunas {
#ifdef REMOTE_ENABLED
	std::expected<syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
	    const std::unique_ptr<lunas::sftp>& src_sftp, const std::unique_ptr<lunas::sftp>& dest_sftp, const struct syncmisc& misc) {
#else
	std::expected<syncstat, lunas::error> copy(const std::string& src, const std::string& dest, const struct syncmisc& misc) {
#endif
#ifdef REMOTE_ENABLED
		if (src_sftp != nullptr || dest_sftp != nullptr)
			return remote::copy(src, dest, src_sftp, dest_sftp, misc);
		else
#endif
			return local::copy(src, dest, misc);
	}
}
