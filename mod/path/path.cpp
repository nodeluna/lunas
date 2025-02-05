module;

#include <string>
#include <filesystem>

export module lunas.path;

export namespace lunas {
	namespace path {
		void append_seperator(std::string& path) noexcept;

		void pop_seperator(std::string& path) noexcept;

		std::string get_lower_dir_level(std::string path);
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
	}
}
