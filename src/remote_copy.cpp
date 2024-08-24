#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <libssh/sftp.h>
#include "remote_copy.h"
#include "local_to_remote.h"
#include "remote_to_local.h"
#include "remote_to_remote.h"
#include "base.h"

namespace fs_remote {
	void copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		std::string count = "(" + std::to_string(base::syncing_counter) + std::string("/") + std::to_string(base::to_be_synced) + ")";
		if(type == DIRECTORY)
			llog::print(count + " [Dir] '" + dest + "'");
		else
			llog::print(count + " [File] '" + dest + "'");

		int copied = 0;
		if(options::dry_run){
			copied = 1;
			goto done;
		}
		if(src_sftp != nullptr && dest_sftp == nullptr)
			copied = remote_to_local::copy(src, dest, src_sftp, type);
		else if(src_sftp == nullptr && dest_sftp != nullptr)
			copied = local_to_remote::copy(src, dest, dest_sftp, type);
		else
			copied = remote_to_remote::copy(src, dest, src_sftp, dest_sftp, type);

done:
		if (copied > 0)
			base::syncing_counter++;
	}
}


#endif // REMOTE_ENABLED
