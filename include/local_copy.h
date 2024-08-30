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

	struct original_name {
		original_name(const std::string& dest, const std::string& original_name,  int& code);
		~original_name();
		private:
			const std::string& lspart;
			const std::string& dest;
			int& synccode;
	};

}

#endif // HANDLING_COPYING_LOCAL
