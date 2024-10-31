#include <string>
#include "os.h"

namespace os {
	void append_seperator(std::string& path) noexcept {
		if (path.empty() != true && path.back() != path_seperator)
			path = path + path_seperator;
	}

	void pop_seperator(std::string& path) noexcept {
		if (path.empty() != true && path.back() == path_seperator)
			path.pop_back();
	}
}
