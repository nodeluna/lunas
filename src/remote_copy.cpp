#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <libssh/sftp.h>
#include "remote_copy.h"

namespace fs_remote {
	void copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		/*if(src_sftp != nullptr && dest_sftp == nullptr)
			remote_to_local::copy(src, dest, src_sftp, type);
		else if(src_sftp == nullptr && dest_sftp != nullptr)
			local_to_remote::copy(src, dest, dest_sftp, type);
		else
			remote_to_remote::copy(src, dest, src_sftp, dest_sftp, type);*/
	}
}


#endif // REMOTE_ENABLED
