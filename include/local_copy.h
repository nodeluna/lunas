#ifndef HANDLING_COPYING_LOCAL
#define HANDLING_COPYING_LOCAL

#include <string>
#include <vector>
#include <future>
#include "copy.h"

#define LOCAL_BUFFER_SIZE 262144

struct lbuffque{
	std::vector<char> buffer;
	long int bytes_read = 0;
	explicit lbuffque(std::uint64_t size) : buffer(size) {}
};

namespace fs_local {
	struct syncstat copy(const std::string& src, const std::string& dest, const short& type);
}

#endif // HANDLING_COPYING_LOCAL
