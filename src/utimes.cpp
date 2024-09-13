#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED
       
#include <string>
#include <expected>
#include <optional>
#include <system_error>
#include <sys/stat.h>
#include <sys/time.h>
#include "utimes.h"
#include "raii_sftp.h"

namespace utime{
	int switch_fill_local(struct time_val& time_val, const struct stat& stats, const short& utime){
		struct timespec timespec;
		int rv;
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

		return 0;
	}


	std::expected<struct time_val, int> lget_local(const std::string& path, const short& utime){
		struct stat stats;
		struct time_val time_val;
		int rv = 0;

		rv = lstat(path.c_str(), &stats);
		if(rv != 0)
			return std::unexpected(-1);

		rv = switch_fill_local(time_val, stats, utime);
		if(rv != 0)
			return std::unexpected(-1);

		return time_val;
	}

	std::expected<struct time_val, int> get_local(const std::string& path, const short& utime){
		struct stat stats;
		struct time_val time_val;
		int rv = 0;

		if(options::follow_symlink == true)
			rv = stat(path.c_str(), &stats);
		else
			rv = lstat(path.c_str(), &stats);

		if(rv != 0)
			return std::unexpected(-1);

		rv = switch_fill_local(time_val, stats, utime);
		if(rv != 0)
			return std::unexpected(-1);

		return time_val;
	}

	std::optional<std::error_code> set_local(const std::string& path, const struct time_val& time_val){
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

		if(rv != 0)
			return std::make_error_code((std::errc) errno);

		return std::nullopt;
	}

#ifdef REMOTE_ENABLED
	int switch_fill_remote(const sftp_attributes& attributes, struct time_val& time_val, const short& utime){
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

		return 0;
	}

	/*std::variant<struct time_val, int> lget_remote(const sftp_session& sftp, const std::string& path, const short& utime){
		sftp_attributes attributes;
		raii::sftp::attributes attr_obj = raii::sftp::attributes(&attributes);
		attributes = sftp_lstat(sftp, path.c_str());
		if(attributes == NULL)
			return -1;

		struct time_val time_val;

		int rv = switch_fill_remote(attributes, time_val, utime);
		if(rv != 0)
			return -1;

		return time_val;
	}*/

	std::expected<struct time_val, int> get_remote(const sftp_session& sftp, const std::string& path, const short& utime){
		sftp_attributes attributes;
		raii::sftp::attributes attr_obj = raii::sftp::attributes(&attributes);
		if(options::follow_symlink)
			attributes = sftp_stat(sftp, path.c_str());
		else
			attributes = sftp_lstat(sftp, path.c_str());
		struct time_val time_val;
		if(attributes == NULL)
			return std::unexpected(-1);

		int rv = switch_fill_remote(attributes, time_val, utime);
		if(rv != 0)
			return std::unexpected(-1);

		return time_val;
	}

	std::optional<SSH_STATUS> set_remote(const sftp_session& sftp, const std::string& path, const struct time_val& time_val){
		struct sftp_attributes_struct attributes;
		attributes.flags = SSH_FILEXFER_ATTR_ACMODTIME;
		attributes.atime = time_val.atime;
		attributes.atime_nseconds = time_val.atime_nsec;
		attributes.mtime = time_val.mtime;
		attributes.mtime_nseconds = time_val.mtime_nsec;

		int rc = sftp_setstat(sftp, path.c_str(), &attributes);
		if(rc != SSH_OK)
			return rc;
			
		return std::nullopt;
	}
	std::optional<SSH_STATUS> lset_remote(const sftp_session& sftp, const std::string& path, const struct time_val& time_val){
		struct sftp_attributes_struct attributes;
		attributes.flags = SSH_FILEXFER_ATTR_ACMODTIME;
		attributes.atime = time_val.atime;
		attributes.atime_nseconds = time_val.atime_nsec;
		attributes.mtime = time_val.mtime;
		attributes.mtime_nseconds = time_val.mtime_nsec;

		int rc = sftp_lsetstat(sftp, path.c_str(), &attributes);
		if(rc != SSH_OK)
			return rc;
			
		return std::nullopt;
	}
#endif // REMOTE_ENABLED
}
