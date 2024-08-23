#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include "remote_to_local.h"
#include "file_types.h"
#include "log.h"


namespace remote_to_remote {
	int copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
		/*if(type == REGULAR_FILE)
			return remote_to_remote::rfile(src, dest, sftp);
		else if(type == DIRECTORY)
			return remote_to_remote::mkdir(src, dest, sftp);
		else if(type == SYMLINK)
			return remote_to_remote::symlink(src, dest, sftp);
		else
			llog::error("can't sync special file '" + src + "'");*/
		return 0;
	}
	int rfile(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
	int mkdir(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
	int symlink(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp);
}

#endif // REMOTE_ENABLED
