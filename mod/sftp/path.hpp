#pragma once

#include <string>
#include <filesystem>

inline auto& path_seperator = std::filesystem::path::preferred_separator;

namespace path
{
	void append_seperator(std::string& path) noexcept;
	void pop_seperator(std::string& path) noexcept;
}
