#pragma once

#include <system_error>
#include <sys/stat.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <expected>
#	include <variant>
#	include <memory>
#	include <filesystem>
#	include <stdexcept>
#	include <cstring>
#	include <utility>
#	include <cstdint>
#endif

#include "sftp/sftp.hpp"
#include "file_types/file_types.hpp"
#include "attributes/attributes.hpp"
#include "error/error.hpp"
#include "stdout/stdout.hpp"

namespace lunas
{
	class attributes {
		private:
			std::variant<std::unique_ptr<lunas::sftp_attributes>, std::pair<struct stat, lunas::file_types>> file_attributes;
			const std::filesystem::path									 file_path;

		public:
			attributes(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path& path,
				   lunas::follow_symlink follow);
			bool		      exists();
			std::string	      name();
			std::filesystem::path path();
			time_t		      mtime();
			lunas::file_types     file_type();
			std::uintmax_t	      file_size();
	};

	std::expected<std::unique_ptr<lunas::attributes>, lunas::error>
	get_attributes(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path& path, lunas::follow_symlink follow);

	std::expected<std::shared_ptr<lunas::attributes>, lunas::error>
	get_attributes_shared(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path& path, lunas::follow_symlink follow);
}
