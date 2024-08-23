#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include "remote_to_remote.h"
#include "file_types.h"
#include "log.h"
#include "raii_sftp.h"


namespace remote_to_remote {
	int copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		/*if(type == REGULAR_FILE)
			return remote_to_remote::rfile(src, dest, src_sftp, dest_sftp);
		else */if(type == DIRECTORY)
			return remote_to_remote::mkdir(src, dest, src_sftp, dest_sftp);
		else if(type == SYMLINK)
			return remote_to_remote::symlink(src, dest, src_sftp, dest_sftp);
		else
			llog::error("can't sync special file '" + src + "'");
		return 0;
	}
	int rfile(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
	int mkdir(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp){
		int rc = sftp::mkdir(dest_sftp, dest);
		if(llog::rc(dest_sftp, dest, rc, "couldn't make directory", NO_EXIT) == false)
			return 0;

		return 1;
	}

	int symlink(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp){
		char* target = sftp_readlink(src_sftp, src.c_str());
		if(target == NULL){
			llog::error("couldn't read symlink '" + src + "', " + ssh_get_error(src_sftp->session));
			return 0;
		}
		raii::sftp::link_target link_obj = raii::sftp::link_target(&target);

		int rc = sftp::symlink(dest_sftp, target, dest);
		if(llog::rc(dest_sftp, dest, rc, "couldn't make symlink", NO_EXIT) == false)
			return 0;

		return 1;
	}
}

#endif // REMOTE_ENABLED
