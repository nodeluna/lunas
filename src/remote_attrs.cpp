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
	bool sync_utimes(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		if(not options::attributes_atime && not options::attributes_mtime)
			return true;

		short utime = 0;
		if(options::attributes_atime)
			utime += ATIME;
		if(options::attributes_mtime)
			utime += MTIME;

		std::expected<struct time_val, int> time_val;
		if(src_sftp != nullptr){
			time_val = utime::get_remote(src_sftp, src, utime);
			if(not time_val){
				llog::error("couldn't get utimes of '" + dest + "', " + ssh_get_error(src_sftp->session));
				return false;
			}
		}else{
			time_val = utime::get_local(src, utime);
			if(not time_val){
				llog::error("couldn't get utimes of '" + dest + "', " + std::strerror(errno));
				return false;
			}
		}

		if(dest_sftp != nullptr){
			std::optional<SSH_STATUS> err;
			if(type == SYMLINK)
				err = utime::lset_remote(dest_sftp, dest, time_val.value());
			else
				err = utime::set_remote(dest_sftp, dest, time_val.value());
			if(err){
				llog::rc(dest_sftp, dest, *err, "couldn't sync utimes", NO_EXIT);
				return false;
			}
		}else{
			auto err = utime::set_local(dest, time_val.value());
			if(err){
				llog::ec(dest, *err, "couldn't sync utimes", NO_EXIT);
				return false;
			}
		}

		return true;
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
