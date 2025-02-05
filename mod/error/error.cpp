module;

#include <string>
#include <string_view>
#include <functional>
#include <libssh/sftp.h>

export module lunas.error;

export namespace lunas {
	enum class error_type {
		none = 0,

		sftp_none		   = SSH_FX_OK,
		sftp_eof		   = SSH_FX_EOF,
		sftp_no_such_file	   = SSH_FX_NO_SUCH_FILE,
		sftp_permission_denied	   = SSH_FX_PERMISSION_DENIED,
		sftp_genaric_failure	   = SSH_FX_FAILURE,
		sftp_bad_message	   = SSH_FX_BAD_MESSAGE,
		sftp_no_connection	   = SSH_FX_NO_CONNECTION,
		sftp_connection_lost	   = SSH_FX_CONNECTION_LOST,
		sftp_operation_unsupported = SSH_FX_OP_UNSUPPORTED,
		sftp_invalid_handle	   = SSH_FX_INVALID_HANDLE,
		sftp_no_such_path	   = SSH_FX_NO_SUCH_PATH,
		sftp_file_already_exists   = SSH_FX_FILE_ALREADY_EXISTS,
		sftp_write_protected	   = SSH_FX_WRITE_PROTECT,
		sftp_no_media		   = SSH_FX_NO_MEDIA,

		ssh_denied = SSH_REQUEST_DENIED,
		ssh_fatal  = SSH_FATAL,
		ssh_other,
		ssh_unknown,

		config_invalid_argument,
		config_invalid_option,
		config_invalid_argument_type,
		config_missing_argument,
		config_missing_option,
		config_preset_name_doesnt_exists,
		config_demo_config_error,
		config_file_parsing_error,
		init_sftp_error,

		presync_same_input_path,

		cppfs_remove,
		cppfs_mkdir,
		cppfs_symlink,
		cppfs_file_size,

		attributes_set_utimes,
		attributes_get_utimes,
		attributes_set_permissions,
		attributes_get_permissions,
		attributes_permissions_check,
		attributes_file_type,
		attributes_symlink_check,
		attributes_space_info,

		ipath,

		local_readdir,
		remote_readdir,

		sftp_readdir,
		sftp_readdir_eof,
	};

	class error {
		private:
			std::string msg	       = "";
			error_type  error_type = error_type::none;

		public:
			error(const std::string_view& message, const enum error_type type);
			error(const enum error_type type);
			error(std::function<void(std::string&, enum error_type&)> custom_constructor);
			error();

			[[nodiscard]] const std::string message() const noexcept;
			[[nodiscard]] enum error_type	value() const noexcept;
	};
}

namespace lunas {
	error::error(const std::string_view& message, const enum error_type type) : msg(message), error_type(type) {
	}

	error::error(const enum error_type type) : error_type(type) {
	}

	error::error(std::function<void(std::string&, enum error_type&)> custom_constructor) {
		custom_constructor(this->msg, this->error_type);
	}

	error::error() {
	}

	const std::string error::message() const noexcept {
		return msg;
	}

	enum error_type error::value() const noexcept {
		return error_type;
	}
}
