#pragma once

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <filesystem>
#endif

inline auto& path_seperator = std::filesystem::path::preferred_separator;

namespace path
{
	void append_seperator(std::string& path) noexcept;
	void pop_seperator(std::string& path) noexcept;
}
