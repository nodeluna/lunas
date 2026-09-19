#pragma once

#include <string>
#include <expected>
#include <variant>
#include <memory>

#include "sftp/sftp.hpp"
#include "file_types/file_types.hpp"
#include "error/error.hpp"
#include "input_path/input_path.hpp"

namespace lunas
{
	std::expected<std::monostate, lunas::error> remove(const std::unique_ptr<lunas::sftp>& sftp, const std::string& path,
							   lunas::file_types file_type, bool dry_run);

	std::expected<std::uintmax_t, lunas::error> get_size_and_remove(const std::unique_ptr<lunas::sftp>& sftp, const std::string& path,
									lunas::file_types file_type, bool dry_run);

	void register_remove(const std::uintmax_t& file_size, lunas::file_types file_type, lunas::ipath::input_path& ipath);

	std::expected<std::monostate, lunas::error> remove_extra(struct lunas::parsed_data& data, const std::string& path,
								 const size_t& src_index, const size_t& dest_index);
}
