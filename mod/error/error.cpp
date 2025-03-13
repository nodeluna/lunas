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

		config_invalid_argument		 = 50,
		config_invalid_option		 = 51,
		config_invalid_argument_type	 = 52,
		config_missing_argument		 = 53,
		config_missing_option		 = 54,
		config_preset_name_doesnt_exists = 55,
		config_demo_config_error	 = 56,
		config_file_parsing_error	 = 57,
		init_sftp_error			 = 58,

		presync_same_input_path = 59,

		cppfs_remove	= 60,
		cppfs_mkdir	= 61,
		cppfs_symlink	= 62,
		cppfs_file_size = 63,

		attributes_get		     = 64,
		attributes_set_utimes	     = 65,
		attributes_get_utimes	     = 66,
		attributes_set_permissions   = 67,
		attributes_get_permissions   = 68,
		attributes_set_ownership     = 69,
		attributes_get_ownership     = 70,
		attributes_permissions_check = 71,
		attributes_file_type	     = 72,
		attributes_symlink_check     = 73,
		attributes_space_info	     = 74,
		attributes_no_such_file	     = sftp_no_such_file,

		ipath		      = 75,
		input_directory_check = 76,
		local_readdir	      = 77,
		remote_readdir	      = 78,
		opendir		      = 79,

		sftp_readdir	 = 80,
		sftp_readdir_eof = 81,
		readdir_eof	 = 82,

		source_not_found      = 83,
		source_broken_symlink = 84,

		dest_check_ok		      = 85,
		dest_check_same_input_patha   = 86,
		dest_check_not_dest	      = 87,
		dest_check_existing_directory = 88,
		dest_check_type_conflict      = 89,
		dest_check_same_mtime	      = 90,
		dest_check_no_space_left      = 91,
		dest_check_brokenlink	      = 92,
		dest_check_orphaned_file      = 93,
		dest_check_skip_sync	      = 94,
		src_check_partial_file	      = 95,

		no_such_file = sftp_no_such_file,

		sync_special_file_ignored = 96,
		sync_is_open		  = 97,
		sync_error_reading	  = 98,
		sync_error_writing	  = 99,
		sync_size_mismatch	  = 100,
		sync_read_symlink	  = 101,
		sync_get_file_size	  = 102,
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
