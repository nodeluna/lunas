#ifndef UTIME
#define UTIME

#ifdef _POSIX_C_SOURCE
#	if _POSIX_C_SOURCE >= 200809L
#		define LUTIMES_EXISTS
#	endif
#elif __ANDROID_API__
#	if __ANDROID_API__ >= 26
#		define LUTIMES_EXISTS
#	endif
#endif

#include "config.h"

#ifdef REMOTE_ENABLED
#	include <libssh/sftp.h>
#	include "raii_sftp.h"
#endif // REMOTE_ENABLED

#include <string>
#include <ctime>
#include <expected>
#include <optional>
#include <system_error>

#define ATIME 1
#define MTIME 2
#define UTIMES 3

struct time_val {
		long atime	= -1;
		long atime_nsec = 0;
		long mtime	= -1;
		long mtime_nsec = 0;
};

namespace utime {
	std::expected<struct time_val, int> lget_local(const std::string& path, const short& utime);
	std::expected<struct time_val, int> get_local(const std::string& path, const short& utime);
	std::optional<std::error_code>	    set_local(const std::string& path, const struct time_val& time_val);

#ifdef REMOTE_ENABLED
	std::expected<struct time_val, int> get_remote(const sftp_session& sftp, const std::string& path, const short& utime);
	std::expected<struct time_val, int> lget_remote(const sftp_session& sftp, const std::string& path, const short& utime);
	std::optional<SSH_STATUS>	    set_remote(const sftp_session& sftp, const std::string& path, const struct time_val& time_val);
	std::optional<SSH_STATUS>	    lset_remote(const sftp_session& sftp, const std::string& path, const struct time_val& time_val);
#endif // REMOTE_ENABLED
}

#endif // UTIME
