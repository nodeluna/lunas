#ifndef LOCAL_TO_LOCAL
#define LOCAL_TO_LOCAL

#include <string>
#include "copy.h"

namespace local_to_local {
	struct syncstat copy(const std::string& src, const std::string& dest, const short& type);
	struct syncstat rfile(const std::string& src, const std::string& dest);
	struct syncstat mkdir(const std::string& src, const std::string& dest);
	struct syncstat symlink(const std::string& src, const std::string& dest);
}

#endif // LOCAL_TO_LOCAL
