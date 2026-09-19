#pragma once

#include <string>
#include <vector>
#include <functional>
#include <expected>
#include <variant>

#include "../options.hpp"
#include "input_path/input_path.hpp"
#include "error/error.hpp"

namespace lunas
{
	namespace cliarg
	{
		struct cliopts {
				std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>> ipaths;
				lunas::config::options									     options;
				std::vector<std::string>								     presets;
		};

		using expect	= std::expected<std::monostate, lunas::error>;
		using options	= lunas::config::options;
		using paths_vec = std::vector<std::variant<struct lunas::ipath::local_path, struct lunas::ipath::remote_path>>;

		std::expected<struct cliopts, lunas::error>
		fillopts(const int& argc, const char* argv[],
			 std::function<expect(const std::string&, options&, paths_vec&)> config_file_preset);
	}
}
