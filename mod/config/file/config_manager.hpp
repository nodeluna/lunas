#pragma once

#include <string>
#include <string>
#include <expected>
#include <cstdlib>
#include <variant>

#include "../options.hpp"
#include "input_path/input_path.hpp"
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
