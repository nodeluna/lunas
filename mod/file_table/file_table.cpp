module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <vector>
#	include <string>
#	include <cstdint>
#endif

export module lunas.file_table;
export import lunas.file_types;

export namespace lunas {
	struct metadata {
			time_t		  mtime	    = 0;
			lunas::file_types file_type = lunas::file_types::not_found;
	};

	struct file_table {
			std::string			     path;
			mutable std::vector<struct metadata> metadatas;

			file_table(const std::string& name_, const struct metadata& metadata, const size_t input_path_index,
			    const size_t files_count)
			    : path(name_) {

				metadatas.resize(files_count);
				metadatas.at(input_path_index) = metadata;
			}

			bool operator==(const file_table& other) const {
				return path == other.path;
			}

			bool operator<(const file_table& other) const {
				return path < other.path;
			}

			bool operator>(const file_table& other) const {
				return path > other.path;
			}
	};
}
