#pragma once

#include <cstdint>
#include <string>

namespace lunas
{
	enum class file_types : uint8_t {
		not_found	    = 0,
		regular_file	    = 1,
		resume_regular_file = 2,
		directory	    = 3,
		symlink		    = 4,
		hardlink	    = 5,
		brokenlink	    = 6,
		socket		    = 7,
		block_file	    = 8,
		character_file	    = 9,
		fifo		    = 10,
		other		    = 11,
	};

	enum class follow_symlink : uint8_t {
		yes,
		no,
	};

	uint8_t	    file_types_to_int(enum file_types type);

	std::string str_file_type(lunas::file_types file_type);

	file_types  if_lspart_return_resume_type(const std::string& path, lunas::file_types type);
}
