#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <filesystem>
#	include <memory>
#	include <expected>
#	include <variant>
#endif

#include <luco.hpp>
#include "error/error.hpp"

namespace lunas
{
	std::expected<luco::node, lunas::error>	    handle_simple_value(const std::string& key, const std::string& value,
									const luco::value_type value_type, const std::string& type_name);

	std::expected<std::monostate, lunas::error> handle_value(const std::string& value, std::pair<std::string, luco::node>& kv_pair);

	std::expected<std::vector<std::pair<std::string, luco::node>>, lunas::error>
	kvoption_parser(const std::string& data, const std::map<std::string, luco::node>& kv_map);
}
