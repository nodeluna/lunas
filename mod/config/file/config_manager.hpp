#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <print>
#	include <string>
#	include <fstream>
#	include <filesystem>
#	include <expected>
#	include <cstdlib>
#	include <cstring>
#	include <cerrno>
#	include <unordered_map>
#	include <map>
#	include <variant>
#	include <algorithm>
#endif

#include "../options_functions.hpp"
#include "../options.hpp"
#include "../filler.hpp"
#include "cppfs/cppfs.hpp"
#include "input_path/input_path.hpp"
#include "stdout/stdout.hpp"
#include "error/error.hpp"

#include <luco.hpp>

namespace lunas
{
	namespace config_file
	{
		inline std::string			    config_dir = std::getenv("HOME") + std::string("/.config/lunas/");
		inline std::string			    file_name  = std::string("lunas.luco");
		std::expected<std::monostate, lunas::error> make_demo_config(lunas::config::options& options);
		std::expected<std::monostate, lunas::error>
		preset(const std::string& name, lunas::config::options& options,
		       std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>& ipaths);
	}
}

#define DEMO_CONFIG                 \
	"global{\n"                 \
	"\t#mkdir = on\n"           \
	"\t#compression = on\n"     \
	"\t#resume = on\n"          \
	"\t#progress = on\n"        \
	"\t#update = on\n"          \
	"\t#minimum-space = 1gib\n" \
	"\t#attributes = mtime\n"   \
	"}\n"                       \
	"luna {\n"                  \
	"\tpath = /path/to/dir1\n"  \
	"\tpath = /path/to/dir2\n"  \
	"\tpath = /path/to/dir3\n"  \
	"\tremote {\n"              \
	"\t\tr = user@ip:dir\n"     \
	"\t\tport = 22\n"           \
	"\t}\n"                     \
	"}\n"
