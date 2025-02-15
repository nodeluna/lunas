module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <expected>
#	include <filesystem>
#	include <cstring>
#	include <cerrno>
#	include <system_error>
#endif

export module lunas.attributes:file_type;
export import lunas.error;
export import lunas.file_types;

export namespace lunas {
	std::expected<lunas::file_types, lunas::error> get_file_type(const std::string& path, lunas::follow_symlink follow);
	lunas::file_types			       get_file_type(const std::filesystem::file_status& status);
	std::expected<bool, lunas::error>	       is_broken_link(const std::string& path);
}

namespace lunas {
	lunas::file_types enum_file_types(std::filesystem::file_status entry) {
		if (entry.type() == std::filesystem::file_type::symlink)
			return lunas::file_types::symlink;
		else if (entry.type() == std::filesystem::file_type::directory)
			return lunas::file_types::directory;
		else if (entry.type() == std::filesystem::file_type::regular)
			return lunas::file_types::regular_file;
		else if (entry.type() == std::filesystem::file_type::socket)
			return lunas::file_types::socket;
		else if (entry.type() == std::filesystem::file_type::not_found)
			return lunas::file_types::not_found;
		else
			return lunas::file_types::other;
	}

	std::expected<lunas::file_types, lunas::error> get_file_type(const std::string& path, lunas::follow_symlink follow) {
		std::error_code		     ec;
		std::filesystem::file_status status;
		if (follow == lunas::follow_symlink::yes)
			status = std::filesystem::status(path, ec);
		else
			status = std::filesystem::symlink_status(path, ec);

		if (ec.value() != 0) {
			std::string	  err  = "couldn't get type of '" + path + "', " + ec.message();
			lunas::error_type type = ec == std::errc::no_such_file_or_directory ? lunas::error_type::attributes_no_such_file
											    : lunas::error_type::attributes_file_type;
			return std::unexpected(lunas::error(err, type));
		}

		return enum_file_types(status);
	}

	lunas::file_types get_file_type(const std::filesystem::file_status& status) {
		return enum_file_types(status);
	}

	std::expected<bool, lunas::error> is_broken_link(const std::string& path) {
		std::error_code ec;
		bool		exists = false;
		std::string	target;

		bool is_symlink = std::filesystem::is_symlink(path, ec);
		if (ec.value() != 0)
			goto err;

		if (not is_symlink)
			return false;

		target = std::filesystem::read_symlink(path, ec);
		if (ec.value() != 0)
			goto err;

		exists = std::filesystem::exists(target, ec);
		if (ec.value() != 0)
			goto err;

		return not exists;
	err:
		std::string err = "couldn't check 'is_broken_link' of '" + path + "', " + ec.message();
		return std::unexpected(lunas::error(err, lunas::error_type::attributes_symlink_check));
	}
}
