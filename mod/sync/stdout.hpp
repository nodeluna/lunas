#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <print>
#	include <cmath>
#endif

#include "types.hpp"
#include "file_types/file_types.hpp"
#include "stdout/stdout.hpp"

namespace lunas
{
	void print_sync(const std::string& src, const std::string& dest, const struct lunas::syncmisc& misc);

	void print_remove_extra(const std::string& path);
}
