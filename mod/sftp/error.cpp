module;

#include <string>
#include <libssh/libssh.h>
#include <libssh/sftp.h>

export module sftp:error;

export namespace lunas {
	enum class sftp_error_type {
		none		      = SSH_FX_OK,
		eof		      = SSH_FX_EOF,
		no_such_file	      = SSH_FX_NO_SUCH_FILE,
		permission_denied     = SSH_FX_PERMISSION_DENIED,
		genaric_failure	      = SSH_FX_FAILURE,
		bad_message	      = SSH_FX_BAD_MESSAGE,
		no_connection	      = SSH_FX_NO_CONNECTION,
		connection_lost	      = SSH_FX_CONNECTION_LOST,
		operation_unsupported = SSH_FX_OP_UNSUPPORTED,
		invalid_handle	      = SSH_FX_INVALID_HANDLE,
		no_such_path	      = SSH_FX_NO_SUCH_PATH,
		file_already_exists   = SSH_FX_FILE_ALREADY_EXISTS,
		write_protected	      = SSH_FX_WRITE_PROTECT,
		no_media	      = SSH_FX_NO_MEDIA,
		ssh_denied	      = SSH_REQUEST_DENIED,
		ssh_fatal	      = SSH_FATAL,
		other		      = 998,
		unknown		      = 999,
	};

	class ssh_error {
		private:
			sftp_error_type error_type = sftp_error_type::other;
			std::string	msg	   = "";

			void register_error_type(int num);
			void register_error_msg(const std::string& message);
			void constructor(const sftp_session& sftp);

		public:
			ssh_error(const sftp_session& sftp);
			ssh_error(const sftp_session& sftp, const std::string& message);
			ssh_error(const std::string& message);
			ssh_error(const ssh_session& ssh);
			ssh_error(const ssh_session& ssh, const std::string& message);
			const std::string& message() const noexcept;
			sftp_error_type	   value() const noexcept;
	};
}

namespace lunas {
	void ssh_error::register_error_type(int num) {
		error_type = static_cast<sftp_error_type>(num);
	}

	void ssh_error::register_error_msg(const std::string& message) {
		msg += message;
	}

	void ssh_error::constructor(const sftp_session& sftp) {
		ssh_session& _ssh_session = sftp->session;

		int num = sftp_get_error(sftp);
		if (num >= 0)
			register_error_type(num);
		else {
			register_error_type(( int ) sftp_error_type::unknown);
			msg = "failed to retrieve sftp server error: ";
		}
		register_error_msg(ssh_get_error(_ssh_session));
	}

	ssh_error::ssh_error(const sftp_session& sftp) {
		constructor(sftp);
	}

	ssh_error::ssh_error(const sftp_session& sftp, const std::string& message) {
		msg += message + ", ";
		constructor(sftp);
	}

	ssh_error::ssh_error(const std::string& message) {
		msg = message;
	}

	ssh_error::ssh_error(const ssh_session& ssh) {
		register_error_msg(ssh_get_error(ssh));
		error_type = static_cast<sftp_error_type>(ssh_get_error_code(ssh));
	}

	ssh_error::ssh_error(const ssh_session& ssh, const std::string& message) {
		msg += message + ", ";
		register_error_msg(ssh_get_error(ssh));
		error_type = static_cast<sftp_error_type>(ssh_get_error_code(ssh));
	}

	const std::string& ssh_error::message() const noexcept {
		return msg;
	}

	sftp_error_type ssh_error::value() const noexcept {
		return error_type;
	}
}
