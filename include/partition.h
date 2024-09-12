#ifndef PARTITION
#define PARTITION

#include <cstdint>

namespace partition{
	bool getting_full(const std::uintmax_t& available_space, const std::uintmax_t& file_size);
}

#endif // PARTITION
