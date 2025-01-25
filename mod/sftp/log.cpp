module;

#include <string_view>
#include <string>
#include <print>
#include <format>

export module lunas.sftp:log;

namespace llog {
	void error(const std::string& data) {
		std::println("\x1b[1;31m-[X] {}\x1b[0m", data);
	}

	void warn(const std::string& data) {
		std::println("{}", data);
	}

	void warn2(const std::string& data) {
		std::print("{}", data);
	}
}

namespace fmt {
	std::string err_path(const std::string_view& error, const std::string_view& path) {
		return std::format("\x1b[1;31m-[X] {} '{}'\x1b[0m\n", error, path);
	}

	std::string err_path(const std::string_view& error, const std::string_view& path, const std::string_view& reason) {
		return std::format("\x1b[1;31m-[X] {} '{}', {}\x1b[0m\n", error, path, reason);
	}

	std::string err_color(const std::string& msg) {
		return std::format("\x1b[1;31m-[X] {}\x1b[0m\n", msg);
	}
}
