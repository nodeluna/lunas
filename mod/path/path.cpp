module;

#include <string>
#include <expected>
#include <filesystem>

export module lunas.path;
export import lunas.error;

export namespace lunas {
	namespace path {
		void append_seperator(std::string& path) noexcept;

		void pop_seperator(std::string& path) noexcept;

		std::string get_lower_dir_level(std::string path);

		std::expected<std::string, lunas::error> resolve_relative_path(std::string path, std::string cwd);

		std::expected<std::string, lunas::error> absolute(std::string path);
	}
}

namespace lunas {
	namespace path {
		void append_seperator(std::string& path) noexcept {
			if (path.empty() != true && path.back() != std::filesystem::path::preferred_separator)
				path = path + std::filesystem::path::preferred_separator;
		}

		void pop_seperator(std::string& path) noexcept {
			if (path.empty() != true && path.back() == std::filesystem::path::preferred_separator)
				path.pop_back();
		}

		std::string get_file_or_dir_name(const std::string& path) {
			if (path.empty())
				return {};
			else if (path.rfind(std::filesystem::path::preferred_separator) == path.npos)
				return path;

			return path.substr(path.rfind(std::filesystem::path::preferred_separator) + 1, path.length());
		}

		std::string get_lower_dir_level(std::string path) {
			if (path.rfind(std::filesystem::path::preferred_separator) == path.npos)
				return path;
			path::pop_seperator(path);
			path.resize(path.length() - get_file_or_dir_name(path).length());
			return path;
		}

		std::expected<std::string, lunas::error> resolve_relative_path(std::string path, std::string cwd) {
			if (not path.empty() && path.size() > 3 && path.substr(0, 3) != "../")
				return path;

			path::pop_seperator(cwd);
			std::string prefix("", 3);
			for (auto itr = path.begin(); itr != path.end(); ++itr) {
				if (prefix.size() >= 3)
					prefix.clear();

				prefix += *itr;

				if (prefix == "../") {
					if (cwd.rfind(std::filesystem::path::preferred_separator) == cwd.npos) {
						return std::unexpected(
						    lunas::error("couldn't resolve relative path '" + path + "' too many '../'"));
					}

					cwd.resize(cwd.rfind(std::filesystem::path::preferred_separator));

					path = path.substr(3, path.size());
					itr  = path.begin();
					prefix.clear();
					prefix += *itr;
				}
			}

			return cwd + std::filesystem::path::preferred_separator + path;
		}

		std::expected<std::string, lunas::error> absolute(std::string path) {
			if (not path.empty() && path.size() > 3 && path.substr(0, 2) == "~/") {
				std::string home = std::getenv("HOME");
				path::append_seperator(home);
				path = home + path.substr(2, path.size());
			}
			auto full_path = resolve_relative_path(path, std::filesystem::current_path().string());
			if (full_path)
				return full_path.value();
			else
				return std::unexpected(full_path.error());
		}
	}
}
