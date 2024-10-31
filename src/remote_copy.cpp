#include "config.h"

#ifdef REMOTE_ENABLED

#	include <string>
#	include <libssh/sftp.h>
#	include "remote_copy.h"
#	include "local_copy.h"
#	include "local_to_remote.h"
#	include "remote_to_local.h"
#	include "remote_to_remote.h"
#	include "base.h"
#	include "remote_attrs.h"
#	include "local_attrs.h"
#	include "log.h"

namespace fs_remote {
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp,
	    const short& type) {
		std::string original_dest = lunas::original_dest(dest);
		llog::print_sync(src, original_dest, type);

		struct syncstat syncstat;

		if (src_sftp != nullptr && dest_sftp == nullptr) {
			syncstat = remote_to_local::copy(src, dest, src_sftp, type);
			struct fs_local::original_name _(dest, original_dest, syncstat.code);
		} else if (src_sftp == nullptr && dest_sftp != nullptr) {
			syncstat = local_to_remote::copy(src, dest, dest_sftp, type);
			struct fs_remote::original_name _(dest_sftp, dest, original_dest, syncstat.code);
		} else {
			syncstat = remote_to_remote::copy(src, dest, src_sftp, dest_sftp, type);
			struct fs_remote::original_name _(dest_sftp, dest, original_dest, syncstat.code);
		}

		if (options::dry_run == false && syncstat.code == 1)
			remote_attrs::sync_utimes(src, original_dest, src_sftp, dest_sftp, type);

		return syncstat;
	}

	original_name::original_name(
	    const sftp_session& session, const std::string& dest_lspart, const std::string& original_name, int& code)
	    : sftp(session), lspart(dest_lspart), dest(original_name), synccode(code) {
	}

	original_name::~original_name() {
		if (options::dry_run)
			return;
		if (synccode != 1 || dest == lspart)
			return;

		int rc = sftp_rename(sftp, lspart.c_str(), dest.c_str());
		if (llog::rc(sftp, lspart, rc, "couldn't rename file to its original name", NO_EXIT) == false)
			synccode = 0;
	}
}

#endif // REMOTE_ENABLED
