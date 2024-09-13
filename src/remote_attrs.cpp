#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <cstring>
#include <cerrno>
#include <any>
#include <libssh/sftp.h>
#include "remote_attrs.h"
#include "utimes.h"
#include "log.h"
#include "file_types.h"
#include "permissions.h"
#include "ownership.h"


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

	bool sync_ownership(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp){
		if(not options::attributes_uid && not options::attributes_gid)
			return true;

		std::any any_own;
		struct own owner;

		if(src_sftp != nullptr){
			any_own = ownership::get_remote(src_sftp, src);
			auto own = std::any_cast<std::expected<struct own, SSH_STATUS>>(any_own);
			if(not own){
				llog::rc(src_sftp, src, own.error(), "couldn't get file ownership", NO_EXIT);
				return false;
			}
			owner = std::any_cast<std::expected<struct own, SSH_STATUS>>(any_own).value();
		}else{
			any_own = ownership::get_local(src);
			auto own = std::any_cast<std::expected<struct own, std::error_code>>(any_own);
			if(not own){
				llog::ec(src, own.error(), "couldn't get file ownership", NO_EXIT);
				return false;
			}
			owner = std::any_cast<std::expected<struct own, std::error_code>>(any_own).value();
		}

		if(not options::attributes_uid)
			owner.uid = -1;
		if(not options::attributes_gid)
			owner.gid = -1;

		if(dest_sftp != nullptr){
			std::optional<SSH_STATUS> err = ownership::set_remote(dest_sftp, dest, owner);
			if(err){
				llog::rc(dest_sftp, dest, *err, "couldn't set file ownership", NO_EXIT); 
				return false;
			}
		}else{
			std::optional<std::error_code> err = ownership::set_local(dest, owner);
			if(err){
				llog::ec(dest, *err, "couldn't set file ownership", NO_EXIT); 
				return false;
			}
		}

		return true;
	}
}

#endif // REMOTE_ENABLED
