#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED
       
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include "utimes.h"
#include "raii_sftp.h"

namespace utime{
	std::variant<struct time_val, int> get_local(const std::string& path, const short& utime){
		struct stat stats;
		struct time_val time_val;
		struct timespec timespec;
		int rv = 0;

		if(options::follow_symlink == true)
			rv = stat(path.c_str(), &stats);
		else
			rv = lstat(path.c_str(), &stats);

		if(rv != 0){
			return -1;
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
				}else
					return -1;
				break;
			case 2:
				timespec = stats.st_mtim;
				time_val.mtime = timespec.tv_sec;
				time_val.mtime_nsec = timespec.tv_nsec;

				rv = clock_gettime(CLOCK_REALTIME, &timespec);
				if(rv == 0){
					time_val.atime = timespec.tv_sec;
					time_val.atime_nsec = timespec.tv_nsec;
				}else
					return -1;
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
		if(time_val.atime == -1 && time_val.mtime == -1)
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

#ifdef REMOTE_ENABLED
	std::variant<struct time_val, int> get_remote(const sftp_session& sftp, const std::string& path, const short& utime){
		sftp_attributes attributes;
		raii::sftp::attributes attr_obj = raii::sftp::attributes(&attributes);
		if(options::follow_symlink)
			attributes = sftp_stat(sftp, path.c_str());
		else
			attributes = sftp_lstat(sftp, path.c_str());
		struct time_val time_val;
		if(attributes == NULL)
			return -1;

		struct timespec timespec;
		int rv;
		switch(utime){
			case 1:
				time_val.atime = attributes->atime;
				time_val.atime_nsec = attributes->atime_nseconds;

				rv = clock_gettime(CLOCK_REALTIME, &timespec);
				if(rv == 0){
					time_val.mtime = timespec.tv_sec;
					time_val.mtime_nsec = timespec.tv_nsec;
				}else
					return -1;
				break;
			case 2:
				time_val.mtime = attributes->mtime;
				time_val.mtime_nsec = attributes->mtime_nseconds;

				rv = clock_gettime(CLOCK_REALTIME, &timespec);
				if(rv == 0){
					time_val.atime = timespec.tv_sec;
					time_val.atime_nsec = timespec.tv_nsec;
				}else
					return -1;
				break;
			case 3:
				time_val.atime = attributes->atime;
				time_val.atime_nsec = attributes->atime_nseconds;
				time_val.mtime = attributes->mtime;
				time_val.mtime_nsec = attributes->mtime_nseconds;
				break;
			default:
				break;
		}

		return time_val;
	}

	int set_remote(const sftp_session& sftp, const std::string& path, const struct time_val& time_val){
		if(time_val.atime == -1 && time_val.mtime == -1)
			return -1;
		struct timeval times[2];
		int rv = 0;

		times[0].tv_sec = time_val.atime;
		times[0].tv_usec = time_val.atime_nsec / 1000;
		times[1].tv_sec = time_val.mtime;
		times[1].tv_usec = time_val.mtime_nsec / 1000;
		rv = sftp_utimes(sftp, path.c_str(), times);

		return rv;
	}

	int sftp_lutimes(const sftp_session& sftp, const std::string& path, const struct time_val& time_val){
		if(time_val.mtime == -1)
			return -1;
		ssh_channel channel = ssh_channel_new(sftp->session);
		int rc = ssh_channel_open_session(channel);
		if(rc != SSH_OK)
			return rc;
		raii::sftp::channel channel_obj = raii::sftp::channel(&channel);

		std::string command = std::string("touch -h -m --date=@") + std::to_string(time_val.mtime) + std::string(".") 
			+ std::to_string(time_val.mtime_nsec) + std::string(" ")+ path;
		rc = ssh_channel_request_exec(channel, command.c_str());
		if(rc != SSH_OK)
			return rc;

		return rc;
	}

#endif // REMOTE_ENABLED
}
