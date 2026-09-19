#pragma once

#include <string>
#include <expected>

#include "error/error.hpp"

namespace lunas
{
	namespace path
	{
		void					 append_seperator(std::string& path) noexcept;

		void					 pop_seperator(std::string& path) noexcept;

		std::string				 parent_directory(std::string path);

		std::expected<std::string, lunas::error> resolve_relative_path(std::string path, std::string cwd);

		std::expected<std::string, lunas::error> absolute(std::string path);
	}
}
