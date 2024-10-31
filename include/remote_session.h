#ifndef REMOTE_SESSION
#define REMOTE_SESSION

#include "config.h"

#ifdef REMOTE_ENABLED

#	include <string>
#	include <libssh/sftp.h>

namespace rsession {
	int	     verify_publickey(const ssh_session& ssh, const std::string& ip);
	int	     auth_password(const ssh_session& ssh, const std::string& ip, const std::string& pw);
	int	     auth_publickey(const ssh_session& ssh, const char* password);
	int	     auth_publickey_passphrase(const ssh_session& ssh, const std::string& ip, const std::string& pw);
	int	     auth_none(const ssh_session& ssh);
	std::string  auth_method(const int& method);
	int	     auth_list(const ssh_session& ssh, const std::string& ip, const std::string& pw);
	ssh_session  init_ssh(const std::string& ip, const int& port, const std::string& pw);
	sftp_session init_sftp(const ssh_session& ssh, const std::string& ip);
	std::string  absolute_path(const sftp_session& sftp, const std::string& ip);
	int	     list_tree(const sftp_session& sftp, const std::string& input_path);
	int	     free_sftp(sftp_session& sftp);
	int	     free_ssh(ssh_session& ssh);
}

#endif // REMOTE_ENABLED

#endif // REMOTE_SESSION
