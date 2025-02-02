module;

#include <filesystem>
#include <string>
#include <expected>
#include <system_error>
#include <variant>

export module lunas.cppfs;

export namespace lunas {
	namespace cppfs {
		std::expected<std::monostate, std::error_code> remove(const std::string& path, bool dry_run);
		std::expected<std::monostate, std::error_code> mkdir(const std::string& path, bool dry_run);
		std::expected<std::monostate, std::error_code> symlink(const std::string& target, const std::string& dest, bool dry_run);
		std::expected<std::uintmax_t, std::error_code> file_size(const std::string& path);

	}
}

namespace lunas {
	namespace cppfs {
		std::expected<std::monostate, std::error_code> remove(const std::string& path, bool dry_run) {
			std::error_code ec;
			if (dry_run == false)
				std::filesystem::remove(path, ec);

			if (ec.value() != 0)
				return std::unexpected(ec);
			return std::monostate();
		}

		std::expected<std::monostate, std::error_code> mkdir(const std::string& path, bool dry_run) {
			std::error_code ec;
			if (dry_run == false)
				std::filesystem::create_directory(path, ec);

			if (ec.value() != 0)
				return std::unexpected(ec);
			return std::monostate();
		}

		std::expected<std::monostate, std::error_code> symlink(const std::string& target, const std::string& dest, bool dry_run) {
			std::error_code ec;
			if (dry_run == false)
				std::filesystem::create_symlink(target, dest, ec);

			if (ec.value() != 0)
				return std::unexpected(ec);
			return std::monostate();
		}

		std::expected<std::uintmax_t, std::error_code> file_size(const std::string& path) {
			std::error_code ec;
			std::uintmax_t	size = std::filesystem::file_size(path, ec);
			if (ec.value() != 0)
				return std::unexpected(ec);
			return size;
		}
	}
}
