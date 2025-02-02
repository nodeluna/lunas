module;

#include <print>
#include <format>
#include <string>
#include <string_view>

export module lunas.stdout;

export namespace lunas {
	template<typename... args_t>
	void printerr(std::format_string<args_t...> fmt, args_t&&... args) {
		std::string output = "\x1b[1;31m-[X] " + std::format(fmt, std::forward<args_t>(args)...) + " \x1b[0m";
		std::println("{}", output);
	}

	template<typename... args_t>
	void warn(std::format_string<args_t...> fmt, args_t&&... args) {
		std::string output = "\x1b[1;31m-[!!] " + std::format(fmt, std::forward<args_t>(args)...) + " \x1b[0m";
		std::println("{}", output);
	}

	template<typename... args_t>
	void println(bool quiet, std::format_string<args_t...> fmt, args_t&&... args) {
		if (not quiet) {
			std::println("{}", std::format(fmt, std::forward<args_t>(args)...));
		}
	}
}
