#pragma once

#include <string>
#include <expected>
#include <variant>
#include <sys/stat.h>
#include <unistd.h>

#include "error/error.hpp"
#include "file_types/file_types.hpp"

namespace lunas
{
	namespace ownership
	{
		struct own {
				int uid = -1;
				int gid = -1;
		};

		std::expected<struct own, lunas::error>	    get(const std::string& path, lunas::follow_symlink follow);
		std::expected<std::monostate, lunas::error> set(const std::string& path, const struct own& own,
								lunas::follow_symlink follow);
	}
}
