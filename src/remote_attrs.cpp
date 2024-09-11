#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <cstring>
#include <cerrno>
#include <libssh/sftp.h>
#include "remote_attrs.h"
#include "utimes.h"
#include "log.h"
#include "file_types.h"
#include "permissions.h"

namespace remote_attrs {
	int sync_utimes(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		std::expected<struct time_val, int> time_val;
		if(src_sftp != nullptr){
			time_val = utime::get_remote(src_sftp, src, UTIMES);
			if(not time_val){
				llog::error("couldn't sync utimes of '" + dest + "', " + ssh_get_error(src_sftp->session));
				return 0;
			}
		}else{
			time_val = utime::get_local(src, UTIMES);
			if(not time_val){
				llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
				return 0;
			}
		}

		if(dest_sftp != nullptr){
			int rc = SSH_OK;
			if(type == SYMLINK)
				rc = utime::sftp_lutimes(dest_sftp, dest, time_val.value());
			else
				rc = utime::set_remote(dest_sftp, dest, time_val.value());
			if(llog::rc(dest_sftp, dest, rc, "couldn't sync utimes", NO_EXIT) == false)
				return 0;
		}else{
			int rv = utime::set_local(dest, time_val.value());
			if(rv != 0){
				llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
				return 0;
			}
		}
		return 1;
	}
}

#endif // REMOTE_ENABLED
