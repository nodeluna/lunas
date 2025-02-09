module;

#include <string>
#include <ctime>
#include <string>
#include <expected>
#include <variant>
#include <cstring>
#include <cstdint>
#include <system_error>
#include <sys/stat.h>
#include <sys/time.h>

export module lunas.attributes:utimes;
export import lunas.file_types;

import lunas.error;

#ifdef _POSIX_C_SOURCE
#	if _POSIX_C_SOURCE >= 200809L
#		define LUTIMES_EXISTS
#	else
#		warning lutimes() wasn't found. modifications to symlinks' mtime/atime will affect their target
#	endif
#elif __ANDROID_API__
#	if __ANDROID_API__ >= 26
#		define LUTIMES_EXISTS
#	endif
#endif

export namespace lunas {
	enum class time_type : uint8_t {
		atime  = 1,
		mtime  = 2,
		utimes = 3,
	};

	struct time_val {
			time_t atime	  = 0;
			time_t atime_nsec = 0;
			time_t mtime	  = 0;
			time_t mtime_nsec = 0;
	};

	namespace utime {
		std::expected<struct time_val, lunas::error> get(
		    const std::string& path, const time_type utime, lunas::follow_symlink follow);

		std::expected<std::monostate, lunas::error> set(
		    const std::string& path, const struct time_val& time_val, lunas::follow_symlink follow);
	}
}

namespace lunas {
	namespace utime {
		int switch_fill_local(struct time_val& time_val, const struct stat& stats, const time_type utime) {
			struct timespec timespec;
			int		rv;
			switch (utime) {
				case time_type::atime:
					timespec	    = stats.st_atim;
					time_val.atime	    = timespec.tv_sec;
					time_val.atime_nsec = timespec.tv_nsec;

					rv = clock_gettime(CLOCK_REALTIME, &timespec);
					if (rv == 0) {
						time_val.mtime	    = timespec.tv_sec;
						time_val.mtime_nsec = timespec.tv_nsec;
					} else
						return -1;
					break;
				case time_type::mtime:
					timespec	    = stats.st_mtim;
					time_val.mtime	    = timespec.tv_sec;
					time_val.mtime_nsec = timespec.tv_nsec;

					rv = clock_gettime(CLOCK_REALTIME, &timespec);
					if (rv == 0) {
						time_val.atime	    = timespec.tv_sec;
						time_val.atime_nsec = timespec.tv_nsec;
					} else
						return -1;
					break;
				case time_type::utimes:
					timespec	    = stats.st_atim;
					time_val.atime	    = timespec.tv_sec;
					time_val.atime_nsec = timespec.tv_nsec;

					timespec	    = stats.st_mtim;
					time_val.mtime	    = timespec.tv_sec;
					time_val.mtime_nsec = timespec.tv_nsec;
					break;
				default:
					break;
			}

			return 0;
		}

		std::expected<struct time_val, lunas::error> get(
		    const std::string& path, const time_type utime, lunas::follow_symlink follow) {
			struct stat	stats;
			struct time_val time_val;
			int		rv = 0;

			if (follow == follow_symlink::yes)
				rv = stat(path.c_str(), &stats);
			else
				rv = lstat(path.c_str(), &stats);

			if (rv != 0) {
				std::string err = "couldn't get utimes of '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::attributes_get_utimes));
			}

			rv = switch_fill_local(time_val, stats, utime);
			if (rv != 0) {
				std::string err = "couldn't get utimes of '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::attributes_get_utimes));
			}

			return time_val;
		}

		std::expected<std::monostate, lunas::error> set(
		    const std::string& path, const struct time_val& time_val, lunas::follow_symlink follow) {
			struct timeval times[2];
			int	       rv = 0;
			times[0].tv_sec	  = time_val.atime;
			times[0].tv_usec  = time_val.atime_nsec / 1000;
			times[1].tv_sec	  = time_val.mtime;
			times[1].tv_usec  = time_val.mtime_nsec / 1000;

			if (follow == follow_symlink::yes)
				rv = utimes(path.c_str(), times);
			else
#ifdef LUTIMES_EXISTS
				rv = lutimes(path.c_str(), times);
#else
				rv = utimes(path.c_str(), times);
#endif // LUTIMES_EXISTS

			if (rv != 0) {
				std::string err = "couldn't set utimes of '" + path + "', " + std::strerror(errno);
				return std::unexpected(lunas::error(err, lunas::error_type::attributes_set_utimes));
			}

			return std::monostate();
		}

	}

}
