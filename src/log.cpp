#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED

#include <string>
#include <cstring>
#include <iostream>

#include "log.h"

namespace llog{
	void error(const std::string& msg) noexcept{
		std::cerr << options::color_error << "-[X] " << msg << options::reset_color << "\n";
	}

	void warn(const std::string& msg) noexcept{
		std::cerr << options::color_error << "-[!!!] " << msg << options::reset_color;
	}

	void print(const std::string& msg) noexcept{
		std::cout << msg << "\n";
	}

#ifdef REMOTE_ENABLED
	void remote_error(const sftp_session& sftp, const std::string& path, const std::string& msg, const int& exit_code){
		remote_error_ssh(sftp->session, path, msg, exit_code);
	}

	void remote_error_ssh(const ssh_session& ssh, const std::string& path, const std::string& msg, const int& exit_code){
		llog::error(msg + " '" + path + "', " + ssh_get_error(ssh));
		if(exit_code != NO_EXIT)
			exit(exit_code);
	}

	bool rc(const sftp_session& sftp, const std::string& path, const int& rc, const std::string& msg, const int& exit_code){
		if(rc != SSH_OK){
			llog::remote_error(sftp, path, msg, exit_code);
			return false;
		}
		return true;
	}
#ifdef REMOTE_ENABLED

	void local_error(const std::string& path, const std::string& msg, const int& exit_code){
		llog::error(msg + " '" + path + "', " + std::strerror(errno));
		if(exit_code != NO_EXIT)
			exit(exit_code);
	}

	bool ec(const std::string& path, const std::error_code& ec, const std::string& msg, const int& exit_code){
		if(ec.value() != 0){
			llog::local_error(path, msg, exit_code);
			return false;
		}
		return true;
	}
}
