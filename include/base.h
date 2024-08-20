#ifndef BASE_DATA_STRUCTURE
#define BASE_DATA_STRUCTURE

#include <string>
#include <vector>
#include <set>
#include "file_types.h"

#define SRC 1
#define DEST 2
#define SRCDEST 3

struct input_path {
	std::string path;
	//short srcdest;
	long int mtime = -1;
	short type = -1;
	bool remote;
};

struct path {
	std::string name;
	mutable std::vector<input_path> input_paths;

	path(const std::string& name_, const struct input_path& input_path) : name(name_) {
		input_paths.push_back(input_path);
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

namespace base {
	inline unsigned long int paths_count = 0;
}

std::set<path>::iterator find_in_tree(const std::set<path>& tree, const std::string& target);

#endif // BASE_DATA_STRUCTURE
