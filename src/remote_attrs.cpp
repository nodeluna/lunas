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

namespace remote_attrs {
	int sync_utimes(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		struct time_val time_val;
		if(src_sftp != nullptr){
			time_val = utime::get_remote(src_sftp, src, UTIMES);
			if(time_val.mtime == -1){
				llog::error("couldn't sync utimes of '" + dest + "', " + ssh_get_error(src_sftp->session));
				return -1;
			}
		}else{
			time_val = utime::get_local(src, UTIMES);
			if(time_val.mtime == -1){
				llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
				return -1;
			}
		}

		if(dest_sftp != nullptr){
			int rc = SSH_OK;
			if(type == SYMLINK)
				rc = utime::sftp_lutimes(dest_sftp, dest, time_val);
			else
				rc = utime::set_remote(dest_sftp, dest, time_val);
			if(llog::rc(dest_sftp, dest, rc, "couldn't sync utimes", NO_EXIT) == false)
				return -1;
		}else{
			int rv = utime::set_local(dest, time_val);
			if(rv != 0){
				llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
				return -1;
			}
		}
		return 0;
	}
}

#endif // REMOTE_ENABLED
