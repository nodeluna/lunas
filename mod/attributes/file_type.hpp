#pragma once

#include <filesystem>
#include <expected>

#include "error/error.hpp"
#include "file_types/file_types.hpp"

namespace lunas
{
	lunas::file_types			       enum_file_types(std::filesystem::file_status entry);

	std::expected<lunas::file_types, lunas::error> get_file_type(const std::string& path, lunas::follow_symlink follow);

	lunas::file_types			       get_file_type(const std::filesystem::file_status& status);

	std::expected<bool, lunas::error>	       is_broken_link(const std::string& path);
}
