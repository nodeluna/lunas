#ifndef REMOTE_SESSION
#define REMOTE_SESSION

#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <libssh/sftp.h>

namespace rsession{
	int verify_publickey(ssh_session& ssh, const std::string& ip);
	ssh_session init_ssh(const std::string& ip, const int& port, const std::string& pw);
	sftp_session init_sftp(const ssh_session& ssh, const std::string& ip);
	std::string absolute_path(const sftp_session& sftp, const std::string& ip);
	int list_tree(const sftp_session& sftp, const std::string& input_path);
	int free_sftp(sftp_session& sftp);
	int free_ssh(ssh_session& ssh);
}

#endif // REMOTE_ENABLED

#endif // REMOTE_SESSION
