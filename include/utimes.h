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

struct time_val{
	long atime = 0;
	long atime_nsec = 0;
	long mtime = 0;
	long mtime_nsec = 0;
};

namespace utime{
	struct time_val get_local(const std::string& path, const short& utime);
	int set_local(const std::string& path, const struct time_val& time_val);

}

#endif // UTIME
