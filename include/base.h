#ifndef BASE_DATA_STRUCTURE
#define BASE_DATA_STRUCTURE

#include <string>
#include <vector>
#include <set>
#include "file_types.h"

#define SRC 1
#define DEST 2
#define SRCDEST 3

#define NON_EXISTENT 0

struct metadata {
		long int mtime = 0;
		short	 type  = NON_EXISTENT;
};

namespace base {
	inline unsigned long int paths_count	 = 0;
	inline unsigned long int to_be_synced	 = 0;
	inline unsigned long int syncing_counter = 1;

	struct path {
			std::string			     name;
			mutable std::vector<struct metadata> metadatas;

			path(const std::string& name_, const struct metadata& metadata, const unsigned long int input_path_index)
			    : name(name_) {
				if (metadatas.empty())
					metadatas.resize(base::paths_count);

				metadatas[input_path_index] = metadata;
			}

			bool operator==(const path& other) const {
				return name == other.name;
			}

			bool operator<(const path& other) const {
				return name < other.name;
			}

			bool operator>(const path& other) const {
				return name > other.name;
			}
	};
}

inline std::set<base::path> content;

inline std::set<base::path> part_files;

#endif // BASE_DATA_STRUCTURE
