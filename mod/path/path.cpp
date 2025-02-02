module;

#include <string>
#include <filesystem>

export module lunas.path;

export namespace lunas {
	namespace path {
		void append_seperator(std::string& path) noexcept;

		void pop_seperator(std::string& path) noexcept;
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
	}
}
