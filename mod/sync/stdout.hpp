#pragma once

#include <string>

#include "types.hpp"

namespace lunas
{
	void print_sync(const std::string& src, const std::string& dest, const struct lunas::syncmisc& misc);

	void print_remove_extra(const std::string& path);
}
