#pragma once

#include <expected>
#include <variant>
#include <memory>
#include <string>

#include "sftp/sftp.hpp"
#include "error/error.hpp"

namespace lunas
{
	namespace file_operations
	{
		std::expected<std::monostate, lunas::error> mkdir(const std::unique_ptr<lunas::sftp>& sftp, const std::string& path,
								  bool dry_run);
	}
}
