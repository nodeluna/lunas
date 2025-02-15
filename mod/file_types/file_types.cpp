module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <cstdint>
#	include <string>
#endif

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

	std::string str_file_type(lunas::file_types file_type) {
		switch (file_type) {
			case lunas::file_types::not_found:
				return "not found";
			case lunas::file_types::regular_file:
				return "regular file";
			case lunas::file_types::resume_regular_file:
				return "resume regular file";
			case lunas::file_types::directory:
				return "directory";
			case lunas::file_types::symlink:
				return "symlink";
			case lunas::file_types::hardlink:
				return "hardlink";
			case lunas::file_types::brokenlink:
				return "brokenlink";
			case lunas::file_types::socket:
				return "socket";
			case lunas::file_types::block_file:
				return "block file";
			case lunas::file_types::character_file:
				return "character file";
			case lunas::file_types::fifo:
				return "fifo";
			default:
				return "other";
		}
	}

	file_types if_lspart_return_resume_type(const std::string& path, lunas::file_types type) {
		if (type == file_types::regular_file && path.size() > 8 && path.substr(path.size() - 8, path.size()) == ".ls.part")
			return lunas::file_types::resume_regular_file;
		else
			return type;
	}
}
