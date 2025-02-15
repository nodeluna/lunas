module;

#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <string_view>
#	include <functional>
#endif

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
		ssh_error  = SSH_ERROR,
		ssh_other,
		ssh_unknown,
		ssh_again,

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

		attributes_get,
		attributes_set_utimes,
		attributes_get_utimes,
		attributes_set_permissions,
		attributes_get_permissions,
		attributes_set_ownership,
		attributes_get_ownership,
		attributes_permissions_check,
		attributes_file_type,
		attributes_symlink_check,
		attributes_space_info,
		attributes_no_such_file = sftp_no_such_file,

		ipath,
		input_directory_check,
		local_readdir,
		remote_readdir,
		opendir,

		sftp_readdir,
		sftp_readdir_eof,
		readdir_eof,

		source_not_found,
		source_broken_symlink,

		dest_check_ok,
		dest_check_same_input_path,
		dest_check_not_dest,
		dest_check_existing_directory,
		dest_check_type_conflict,
		dest_check_same_mtime,
		dest_check_no_space_left,
		dest_check_brokenlink,
		dest_check_orphaned_file,
		dest_check_skip_sync,
		src_check_partial_file,

		no_such_file = sftp_no_such_file,

		sync_special_file_ignored,
		sync_is_open,
		sync_error_reading,
		sync_error_writing,
		sync_size_mismatch,
		sync_read_symlink,
		sync_get_file_size,
	};

	class error {
		private:
			std::string msg	 = "";
			error_type  type = error_type::none;

		public:
			error(const std::string_view& message, const enum error_type type);
			error(const std::string& message);
			error(const enum error_type type);
			error(std::function<void(std::string&, enum error_type&)> custom_constructor);
			error();

			[[nodiscard]] const std::string message() const noexcept;
			[[nodiscard]] const char*	what() const;
			[[nodiscard]] enum error_type	value() const noexcept;
	};
}

namespace lunas {
	error::error(const std::string_view& message, const enum error_type type) : msg(message), type(type) {
	}

	error::error(const std::string& message) : msg(message) {
	}

	error::error(const enum error_type type) : type(type) {
	}

	error::error(std::function<void(std::string&, enum error_type&)> custom_constructor) {
		custom_constructor(this->msg, this->type);
	}

	error::error() {
	}

	const std::string error::message() const noexcept {
		return msg;
	}

	const char* error::what() const {
		return msg.c_str();
	}

	enum error_type error::value() const noexcept {
		return type;
	}
}
