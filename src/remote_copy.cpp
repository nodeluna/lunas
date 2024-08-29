#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <libssh/sftp.h>
#include "remote_copy.h"
#include "local_to_remote.h"
#include "remote_to_local.h"
#include "remote_to_remote.h"
#include "base.h"
#include "remote_attrs.h"
#include "local_attrs.h"
#include "log.h"


namespace fs_remote {
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		llog::print_sync(src, dest, type);

		struct syncstat syncstat;
		if(src_sftp != nullptr && dest_sftp == nullptr)
			syncstat = remote_to_local::copy(src, dest, src_sftp, type);
		else if(src_sftp == nullptr && dest_sftp != nullptr)
			syncstat = local_to_remote::copy(src, dest, dest_sftp, type);
		else
			syncstat = remote_to_remote::copy(src, dest, src_sftp, dest_sftp, type);

		if(options::dry_run == false && syncstat.code == 1)
			remote_attrs::sync_utimes(src, dest, src_sftp, dest_sftp, type); 

		return syncstat;
	}
}


#endif // REMOTE_ENABLED
