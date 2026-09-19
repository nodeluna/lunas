#pragma once

#include <string>
#include <expected>
#include <variant>
#include <sys/stat.h>
#include <sys/time.h>
#include <ctime>

#include "file_types/file_types.hpp"
#include "error/error.hpp"

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

namespace lunas
{
	enum class time_type {
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

	namespace utime
	{
		int switch_fill_local(struct time_val& time_val, const struct stat& stats, const time_type utime);

		std::expected<struct time_val, lunas::error> get(const std::string& path, const time_type utime,
								 lunas::follow_symlink follow);

		std::expected<std::monostate, lunas::error>  set(const std::string& path, const struct time_val& time_val,
								 lunas::follow_symlink follow);
	}
}
