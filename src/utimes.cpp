#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include "utimes.h"

namespace utime{
	struct time_val get_local(const std::string& path, const short& utime){
		struct stat stats;
		struct time_val time_val;
		struct timespec timespec;
		int rv = 0;

		if(options::follow_symlink == true)
			rv = stat(path.c_str(), &stats);
		else
			rv = lstat(path.c_str(), &stats);

		if(rv != 0){
			return time_val;
		}

		switch(utime){
			case 1:
				timespec = stats.st_atim;
				time_val.atime = timespec.tv_sec;
				time_val.atime_nsec = timespec.tv_nsec;

				rv = clock_gettime(CLOCK_REALTIME, &timespec);
				if(rv == 0){
					time_val.mtime = timespec.tv_sec;
					time_val.mtime_nsec = timespec.tv_nsec;
				}
				break;
			case 2:
				timespec = stats.st_mtim;
				time_val.mtime = timespec.tv_sec;
				time_val.mtime_nsec = timespec.tv_nsec;

				rv = clock_gettime(CLOCK_REALTIME, &timespec);
				if(rv == 0){
					time_val.atime = timespec.tv_sec;
					time_val.atime_nsec = timespec.tv_nsec;
				}
				break;
			case 3:
				timespec = stats.st_atim;
				time_val.atime = timespec.tv_sec;
				time_val.atime_nsec = timespec.tv_nsec;

				timespec = stats.st_mtim;
				time_val.mtime = timespec.tv_sec;
				time_val.mtime_nsec = timespec.tv_nsec;
				break;
			default:
				break;
		}

		return time_val;
	}

	int set_local(const std::string& path, const struct time_val& time_val){
		if(time_val.atime == 0 && time_val.mtime == 0)
			return -1;
		struct timeval times[2];
		int rv = 0;
		times[0].tv_sec = time_val.atime;
		times[0].tv_usec = time_val.atime_nsec / 1000;
		times[1].tv_sec = time_val.mtime;
		times[1].tv_usec = time_val.mtime_nsec / 1000;
		if(options::follow_symlink == true)
			rv = utimes(path.c_str(), times);
		else
#ifdef LUTIMES_EXISTS
			rv = lutimes(path.c_str(), times);
#else
			rv = utimes(path.c_str(), times);
#endif // LUTIMES_EXISTS

		return rv;
	}
}
