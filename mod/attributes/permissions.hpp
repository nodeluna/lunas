#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <filesystem>
#	include <expected>
#	include <variant>
#	include <cstring>
#	include <system_error>
#	include <fcntl.h>
#	include <sys/stat.h>
#endif

#include "error/error.hpp"
#include "file_types/file_types.hpp"

namespace fs = std::filesystem;

namespace lunas
{
	namespace permissions
	{
		std::expected<bool, lunas::error>		    is_file_readable(const std::string& path, lunas::follow_symlink follow);

		std::expected<std::filesystem::perms, lunas::error> get(const std::string& path, lunas::follow_symlink follow);

		std::expected<std::monostate, lunas::error>	    set(const std::string& path, std::filesystem::perms permissions,
									lunas::follow_symlink follow);

		std::expected<std::monostate, lunas::error>	    set(const std::string& path, unsigned int permissions,
									lunas::follow_symlink follow);
	}
}
