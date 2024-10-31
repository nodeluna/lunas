#ifndef OS_PLATFORM
#define OS_PLATFORM

#ifdef _WIN32
#	define path_seperator '\\'
#else
#	define path_seperator '/'
#endif

#include <string>

namespace os {
	void append_seperator(std::string& path) noexcept;

	void pop_seperator(std::string& path) noexcept;
}

#endif // OS_PLATFORM
