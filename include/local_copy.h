#ifndef HANDLING_COPYING_LOCAL
#define HANDLING_COPYING_LOCAL

#include <string>
#include "copy.h"


namespace fs_local {
	struct syncstat copy(const std::string& src, const std::string& dest, const short& type);
}

#endif // HANDLING_COPYING_LOCAL
