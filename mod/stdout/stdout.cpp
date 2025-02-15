module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <print>
#	include <format>
#	include <string>
#	include <string_view>
#endif

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
	void warn_ok(std::format_string<args_t...> fmt, args_t&&... args) {
		std::string output = "-[!!] " + std::format(fmt, std::forward<args_t>(args)...);
		std::println("{}", output);
	}

	template<typename... args_t>
	void println(bool quiet, std::format_string<args_t...> fmt, args_t&&... args) {
		if (not quiet) {
			std::string output = std::format(fmt, std::forward<args_t>(args)...);
			std::println("{}", output);
		}
	}

	template<typename... args_t>
	std::string fmterr(std::format_string<args_t...> fmt, args_t&&... args) {
		return "\x1b[1;31m-[X] " + std::format(fmt, std::forward<args_t>(args)...) + " \x1b[0m\n";
	}

	template<typename... args_t>
	void fmterr_multiline(std::string& err, std::format_string<args_t...> fmt, args_t&&... args) {
		bool empty = false;
		if (not err.empty()) {
			err += "\x1b[1;31m-[X] ";
			empty = true;
		}
		err += std::format(fmt, std::forward<args_t>(args)...);
		if (not empty)
			err += " \x1b[0m";
		err += "\n";
	}

	template<typename... args_t>
	std::string fmtwarn(std::format_string<args_t...> fmt, args_t&&... args) {
		return "\x1b[1;31m-[!!] " + std::format(fmt, std::forward<args_t>(args)...) + " \x1b[0m\n";
	}
}
