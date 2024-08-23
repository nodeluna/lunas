#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include "remote_to_local.h"
#include "file_types.h"
#include "log.h"
#include "cppfs.h"
#include "raii_sftp.h"


namespace remote_to_local {
	int copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const short& type){
		/*if(type == REGULAR_FILE)
			return remote_to_local::rfile(src, dest, sftp);
		else*/ if(type == DIRECTORY)
			return remote_to_local::mkdir(src, dest, sftp);
		else if(type == SYMLINK)
			return remote_to_local::symlink(src, dest, sftp);
		else
			llog::error("can't sync special file '" + src + "'");
		return 0;
	}
	int rfile(const std::string& src, const std::string& dest, const sftp_session& sftp);
	int mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp){
		std::error_code ec;
		cppfs::mkdir(dest, ec);
		if(llog::ec(dest, ec, "couldn't make directory", NO_EXIT) == false)
			return 0;

		return 1;
	}
	int symlink(const std::string& src, const std::string& dest, const sftp_session& sftp){
		std::error_code ec;
		char* target = sftp_readlink(sftp, src.c_str());
		raii::sftp::link_target link_obj = raii::sftp::link_target(&target);
		cppfs::symlink(target, dest, ec);
		if(llog::ec(dest, ec, "couldn't make symlink", NO_EXIT) == false)
			return 0;

		return 1;
	}
}

#endif // REMOTE_ENABLED
