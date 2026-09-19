#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <expected>
#	include <variant>
#	include <memory>
#	include <string>
#endif

#include "sftp/sftp.hpp"
#include "file_types/file_types.hpp"
#include "error/error.hpp"
#include "attributes/attributes.hpp"
#include "cppfs/cppfs.hpp"

namespace lunas
{
	namespace file_operations
	{
		std::expected<std::monostate, lunas::error> mkdir(const std::unique_ptr<lunas::sftp>& sftp, const std::string& path,
								  bool dry_run);
	}
}
