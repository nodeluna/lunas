module;

#include <libssh/libssh.h>
#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <typeinfo>
#endif

export module lunas.sftp:error;
export import lunas.error;

export namespace lunas {
	lunas::error ssh_error(const sftp_session& sftp);
	lunas::error ssh_error(const sftp_session& sftp, const std::string& message);
	lunas::error ssh_error(const ssh_session& ssh);
	lunas::error ssh_error(const std::string& msg);
	lunas::error ssh_error(const ssh_session& ssh, const std::string& message);
}

namespace lunas {
	void sftp_error_filling(const sftp_session& sftp, std::string& msg, lunas::error_type& type) {
		ssh_session& _ssh_session = sftp->session;

		int num = sftp_get_error(sftp);
		if (num >= 0)
			type = static_cast<lunas::error_type>(num);
		else {
			type = lunas::error_type::ssh_unknown;
			msg  = "failed to retrieve sftp server error: ";
		}
		msg += ssh_get_error(_ssh_session);
	}

	lunas::error ssh_error(const sftp_session& sftp) {
		std::string	  err_msg    = "";
		lunas::error_type error_type = lunas::error_type::none;
		sftp_error_filling(sftp, err_msg, error_type);

		auto error_constructor = [&](std::string& msg, lunas::error_type& type) {
			msg  = err_msg;
			type = error_type;
		};

		return lunas::error(error_constructor);
	}

	lunas::error ssh_error(const sftp_session& sftp, const std::string& message) {
		std::string	  err_msg    = message + ", ";
		lunas::error_type error_type = lunas::error_type::none;
		sftp_error_filling(sftp, err_msg, error_type);

		auto error_constructor = [&](std::string& msg, lunas::error_type& type) {
			msg  = err_msg;
			type = error_type;
		};

		return lunas::error(error_constructor);
	}

	lunas::error ssh_error(const ssh_session& ssh) {
		std::string	  err_msg    = ssh_get_error(ssh);
		lunas::error_type error_type = static_cast<lunas::error_type>(ssh_get_error_code(ssh));

		auto error_constructor = [&](std::string& msg, lunas::error_type& type) {
			msg  = err_msg;
			type = error_type;
		};

		return lunas::error(error_constructor);
	}

	lunas::error ssh_error(const ssh_session& ssh, const std::string& message) {
		std::string	  err_msg    = message + ", " + ssh_get_error(ssh);
		lunas::error_type error_type = static_cast<lunas::error_type>(ssh_get_error_code(ssh));

		auto error_constructor = [&](std::string& msg, lunas::error_type& type) {
			msg  = err_msg;
			type = error_type;
		};

		return lunas::error(error_constructor);
	}

	lunas::error ssh_error(const std::string& err_msg) {
		auto error_constructor = [&](std::string& msg, lunas::error_type& type) {
			msg  = err_msg;
			type = lunas::error_type::ssh_other;
		};

		return lunas::error(error_constructor);
	}
}
