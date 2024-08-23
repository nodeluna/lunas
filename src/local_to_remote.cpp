#include "config.h"

#ifdef REMOTE_ENABLED

#include <libssh/sftp.h>
#include <string>
#include <fstream>
#include <filesystem>
#include "local_to_remote.h"
#include "raii_sftp.h"
#include "log.h"
#include "file_types.h"

namespace fs = std::filesystem;

namespace local_to_remote {
	int copy(const std::string& src, const std::string& dest, const sftp_session& sftp, const short& type){
		/*if(type == REGULAR_FILE)
			return local_to_remote::rfile(src, dest, sftp);
		else */if(type == DIRECTORY)
			return local_to_remote::mkdir(src, dest, sftp);
		else if(type == SYMLINK)
			return local_to_remote::symlink(src, dest, sftp);
		else
			llog::error("can't sync special file '" + src + "'");

		return 0;
	}

	int rfile(const std::string& src, const std::string& dest, const sftp_session& sftp);

	int mkdir(const std::string& src, const std::string& dest, const sftp_session& sftp){
		int rc = sftp::mkdir(sftp, dest);
		if(llog::rc(sftp, dest, rc, "couldn't make directory", NO_EXIT) == false)
			return 0;

		return 1;
	}

	int symlink(const std::string& src, const std::string& dest, const sftp_session& sftp){
		std::error_code ec;
		std::string target = fs::read_symlink(src, ec);
		if(llog::ec(src, ec, "couldn't read symlink", NO_EXIT) == false)
			return 0;

		int rc = sftp::symlink(sftp, target, dest);
		if(llog::rc(sftp, dest, rc, "couldn't make symlink", NO_EXIT) == false)
			return 0;

		return 1;
	}
}


#endif // REMOTE_ENABLED
