#ifndef UTIME
#define UTIME

#ifdef _POSIX_C_SOURCE
	#if _POSIX_C_SOURCE >= 200809L
		#define LUTIMES_EXISTS
	#endif
#elif __ANDROID_API__
	#if __ANDROID_API__ >= 26
		#define LUTIMES_EXISTS
	#endif
#endif

#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED

#include <string>
#include <ctime>
#include <variant>

#define ATIME 1
#define MTIME 2
#define UTIMES 3

struct time_val{
	long atime = -1;
	long atime_nsec = 0;
	long mtime = -1;
	long mtime_nsec = 0;
};

namespace utime{
	std::variant<struct time_val, int> get_local(const std::string& path, const short& utime);
	int set_local(const std::string& path, const struct time_val& time_val);

#ifdef REMOTE_ENABLED
	std::variant<struct time_val, int> get_remote(const sftp_session& sftp, const std::string& path, const short& utime);
	int set_remote(const sftp_session& sftp, const std::string& path, const struct time_val& time_val);
	int sftp_lutimes(const sftp_session& sftp, const std::string& path, const struct time_val& time_val);
#endif // REMOTE_ENABLED
}

#endif // UTIME
