module;

#include <cstdint>

export module lunas.file_types;

export namespace lunas {
	enum class file_types : uint8_t {
		not_found = 0,
		regular_file,
		resume_regular_file,
		directory,
		symlink,
		hardlink,
		brokenlink,
		socket,
		block_file,
		character_file,
		fifo,
		other,
	};

	enum class follow_symlink : uint8_t {
		yes,
		no,
	};
}
