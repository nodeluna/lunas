#include <string>
#include "os.h"

namespace os{
	void append_seperator(std::string& path) noexcept{
		if(path.empty() != true && path.back() != os_platform_seperator)
			path = path + os_platform_seperator;
	}
	void pop_seperator(std::string& path) noexcept{
		if(path.empty() != true && path.back() == os_platform_seperator)
			path.pop_back();
	}
}
