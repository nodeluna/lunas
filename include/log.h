#ifndef LUNA_LOG
#define LUNA_LOG

#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED

#include <string>
#include <cstdlib>
#include <system_error>

#define NO_EXIT -1

namespace llog{
	void error(const std::string& msg) noexcept;

	void error_exit(const std::string& msg, const int& code);

	void warn(const std::string& msg) noexcept;

	void print(const std::string& msg) noexcept;

#ifdef REMOTE_ENABLED
	void remote_error(const sftp_session& sftp, const std::string& path, const std::string& msg, const int& exit_code);

	void remote_error_ssh(const ssh_session& ssh, const std::string& path, const std::string& msg, const int& exit_code);

	bool rc(const sftp_session& sftp, const std::string& path, const int& rc, const std::string& msg, const int& exit_code);
#endif // REMOTE_ENABLED

	void local_error(const std::string& path, const std::string& msg, const int& exit_code);

	bool ec(const std::string& path, const std::error_code& ec, const std::string& msg, const int& exit_code);

	void print_sync(const std::string& src, const std::string& dest, const short& type);
}

#endif // LUNA_LOG
