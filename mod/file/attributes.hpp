#pragma once

#include <sys/stat.h>

#include <string>
#include <expected>
#include <variant>
#include <memory>
#include <filesystem>
#include <utility>
#include <cstdint>

#include "sftp/sftp.hpp"
#include "file_types/file_types.hpp"
#include "error/error.hpp"

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
