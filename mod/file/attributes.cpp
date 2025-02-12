module;

#include <string>
#include <expected>
#include <variant>
#include <memory>
#include <filesystem>
#include <stdexcept>
#include <cstring>
#include <sys/stat.h>
#include <utility>

export module lunas.file:attributes;

import lunas.sftp;
import lunas.file_types;
import lunas.attributes;
import lunas.error;

import lunas.stdout;

export namespace lunas {
	class attributes {
		private:
			std::variant<std::unique_ptr<lunas::sftp_attributes>, std::pair<struct stat, lunas::file_types>> file_attributes;
			const std::filesystem::path									 file_path;

		public:
			attributes(
			    const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path& path, lunas::follow_symlink follow);
			bool		  exists();
			std::string	  name();
			std::string	  path();
			time_t		  mtime();
			lunas::file_types file_type();
	};

	std::expected<std::unique_ptr<lunas::attributes>, lunas::error> get_attributes(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path& path, lunas::follow_symlink follow);
}

namespace lunas {
	attributes::attributes(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path& path, lunas::follow_symlink follow)
	    : file_path(path) {
		if (sftp != nullptr) {
			auto attr = sftp->attributes(path, follow);
			if (not attr)
				throw attr.error();

			file_attributes = std::move(attr.value());
		} else {
			struct stat stats;
			int	    rc = 0;
			if (follow == lunas::follow_symlink::yes)
				rc = stat(path.c_str(), &stats);
			else
				rc = lstat(path.c_str(), &stats);
			if (rc != 0) {
				lunas::error_type error_type =
				    errno == ENOENT ? lunas::error_type::no_such_file : lunas::error_type::attributes_get;
				std::string err = "couldn't get attributes of '" + path.string() + "', " + std::strerror(errno);
				throw lunas::error(err, error_type);
			}
			auto type = lunas::get_file_type(path, follow);
			if (not type)
				throw type.error();

			std::pair<struct stat, lunas::file_types> pair;
			{
				pair.first  = std::move(stats);
				pair.second = std::move(type.value());
			}

			file_attributes = std::move(pair);
		}
	}

	std::string attributes::name() {
		if (std::holds_alternative<std::unique_ptr<lunas::sftp_attributes>>(file_attributes)) {
			auto& attr = std::get<std::unique_ptr<lunas::sftp_attributes>>(file_attributes);
			return attr->name();
		} else {
			return file_path.filename();
		}
	}

	std::string attributes::path() {
		if (std::holds_alternative<std::unique_ptr<lunas::sftp_attributes>>(file_attributes)) {
			auto& attr = std::get<std::unique_ptr<lunas::sftp_attributes>>(file_attributes);
			return attr->path();
		} else {
			return file_path;
		}
	}

	time_t attributes::mtime() {
		if (std::holds_alternative<std::unique_ptr<lunas::sftp_attributes>>(file_attributes)) {
			auto& attr = std::get<std::unique_ptr<lunas::sftp_attributes>>(file_attributes);
			return attr->mtime();
		} else {
			auto& attr = std::get<std::pair<struct stat, lunas::file_types>>(file_attributes);
			return attr.first.st_mtim.tv_sec;
		}
	}

	lunas::file_types attributes::file_type() {
		if (std::holds_alternative<std::unique_ptr<lunas::sftp_attributes>>(file_attributes)) {
			auto& attr = std::get<std::unique_ptr<lunas::sftp_attributes>>(file_attributes);
			return lunas::if_lspart_return_resume_type(attr->path(), attr->file_type());
		} else {
			auto& attr = std::get<std::pair<struct stat, lunas::file_types>>(file_attributes);
			return lunas::if_lspart_return_resume_type(file_path, attr.second);
		}
	}

	std::expected<std::unique_ptr<lunas::attributes>, lunas::error> get_attributes(
	    const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path& path, lunas::follow_symlink follow) {
		try {
			return std::make_unique<lunas::attributes>(sftp, path, follow);
		} catch (const lunas::error& e) {
			return std::unexpected(e);
		}
	}
}
