#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <filesystem>
#	include <string>
#	include <expected>
#	include <system_error>
#	include <variant>
#	include <cerrno>
#	include <cstring>
#endif

#include "error/error.hpp"

namespace lunas
{
	namespace cppfs
	{
		std::expected<std::monostate, lunas::error> remove(const std::string& path, bool dry_run);
		std::expected<std::monostate, lunas::error> mkdir(const std::string& path, bool dry_run);
		std::expected<std::monostate, lunas::error> symlink(const std::string& target, const std::string& dest, bool dry_run);
		std::expected<std::uintmax_t, lunas::error> file_size(const std::string& path);
	}
}
