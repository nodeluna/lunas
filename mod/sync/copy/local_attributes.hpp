#pragma once

#include <string>
#include <expected>
#include <variant>

#include "../types.hpp"
#include "file_types/file_types.hpp"
#include "error/error.hpp"
#include "attributes/attributes.hpp"

namespace lunas
{
	namespace ownership
	{
		std::expected<std::monostate, lunas::error> local_to_local(const std::string& src, const std::string& dest,
									   const syncmisc& misc);
	}

	namespace utimes
	{
		std::expected<std::monostate, lunas::error> local(const std::string& src, const std::string& dest, const syncmisc& misc);
	}

	namespace permissions
	{
		std::expected<std::monostate, lunas::error> local_to_local(const std::string& src, const std::string& dest,
									   const syncmisc& misc);
	}
}
