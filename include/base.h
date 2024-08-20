#ifndef BASE_DATA_STRUCTURE
#define BASE_DATA_STRUCTURE

#include <string>
#include <vector>
#include <set>
#include "file_types.h"

#define SRC 1
#define DEST 2
#define SRCDEST 3

namespace base {
	inline unsigned long int paths_count = 0;
}

#define NON_EXISTENT -1

struct metadata {
	long int mtime = NON_EXISTENT;
	short type = -1;
};

struct path {
	std::string name;
	mutable std::vector<struct metadata> metadatas;

	path(const std::string& name_, const struct metadata& metadata, const unsigned long int input_path_index) : name(name_) {
		if(metadatas.empty())
			metadatas.resize(base::paths_count);
		
		metadatas[input_path_index] = metadata;
	}

	bool operator==(const path& other) const{
		return name == other.name;
	}

	bool operator<(const path& other) const{
		return name < other.name;
	}
	bool operator>(const path& other) const{
		return name > other.name;
	}
};


inline std::set<path> content;

std::set<path>::iterator find_in_tree(const std::set<path>& tree, const std::string& target);

#endif // BASE_DATA_STRUCTURE
