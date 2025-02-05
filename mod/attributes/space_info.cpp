module;

#include <string>
#include <filesystem>
#include <expected>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <system_error>

export module lunas.attributes:space_info;
export import lunas.error;
import lunas.path;

namespace fs = std::filesystem;

export namespace lunas {
	std::expected<std::uintmax_t, lunas::error> available_space(const std::string& path) {
		std::error_code ec;
		auto		err_func = [](const std::string& path) -> lunas::error {
			   std::string err = "couldn't check partition space of '" + path + "', " + std::strerror(errno);
			   return lunas::error(err, lunas::error_type::attributes_space_info);
		};

		if (fs::exists(path, ec) == false) {
			if (ec.value() != 0)
				return std::unexpected(err_func(path));

			std::string lower_path = lunas::path::get_lower_dir_level(path);
			if (fs::exists(lower_path, ec) == false)
				return std::unexpected(err_func(lower_path));

			fs::space_info availabe_space = fs::space(lower_path, ec);
			if (ec.value() != 0)
				return std::unexpected(err_func(lower_path));

			return availabe_space.available;
		}

		fs::space_info availabe_space = fs::space(path, ec);
		if (ec.value() != 0)
			return std::unexpected(err_func(path));

		return availabe_space.available;
	}
}
