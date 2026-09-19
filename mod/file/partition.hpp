#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <filesystem>
#	include <memory>
#	include <expected>
#	include <variant>
#	include <system_error>
#endif

#include "sftp/sftp.hpp"
#include "error/error.hpp"
#include "path/path.hpp"

namespace lunas
{
	class partition {
		private:
			std::variant<std::unique_ptr<lunas::sftp_partition>, std::filesystem::space_info> _partition;

		public:
			partition(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path);
			std::uintmax_t available();
			std::uintmax_t capacity();
	};

	std::expected<std::unique_ptr<lunas::partition>, lunas::error> get_partition(const std::unique_ptr<lunas::sftp>& sftp,
										     const std::filesystem::path&	 path);
}
