#include <cstdint>
#include "partition.h"
#include "config.h"

namespace partition {
	bool getting_full(const std::uintmax_t& available_space, const std::uintmax_t& file_size) {
		std::uintmax_t size_left = available_space - file_size;
		if (file_size > available_space)
			return true;
		else if (size_left <= options::minimum_space)
			return true;
		return false;
	}
}
