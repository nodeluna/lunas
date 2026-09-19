#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <vector>
#	include <memory>
#	include <variant>
#endif

#include "file/file.hpp"

namespace lunas
{
	enum class hook_action {
		sync,
		dont_sync,
	};
}
