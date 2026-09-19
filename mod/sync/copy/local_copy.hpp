#pragma once

#include <expected>
#include <string>

#include "../types.hpp"
#include "../stdout.hpp"
#include "local_attributes.hpp"
#include "local_to_local.hpp"
#include "error/error.hpp"

namespace lunas
{
	namespace local
	{
		std::expected<lunas::syncstat, lunas::error> copy(const std::string& src, const std::string& dest,
								  const lunas::syncmisc& misc);
	}
}
