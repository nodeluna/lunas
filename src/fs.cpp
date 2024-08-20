#include <exception>
#include <filesystem>
#include <string>
#include <iostream>
#include <set>
#include "base.h"
#include "fs.h"
#include "file_types.h"
#include "utimes.h"
#include "log.h"

namespace fs = std::filesystem;

int list_tree(const struct local_path& local_path){
	const std::string& dir_path = local_path.path;

	if(status::local_type(dir_path, true) != DIRECTORY){
		log::warn("path '" + dir_path + "' not a directory");
		return 1;
	}

	try {
		for(const auto& entry : fs::recursive_directory_iterator(dir_path, fs::directory_options::skip_permission_denied)){
			std::string str_entry = entry.path().string();
			struct input_path input_path;

			input_path.path = local_path.path;
			input_path.srcdest = local_path.srcdest;

			short type = status::local_type(str_entry, false);
			if(type == -1)
				llog::local_error(str_entry, "couldn't get type of", EXIT_FAILURE);
			input_path.type = type;

			struct time_val time_val = utime::get_local(str_entry, 2);
			if(time_val.mtime == 0)
				llog::local_error(str_entry, "couldn't get mtime of file", EXIT_FAILURE);
			input_path.mtime = time_val.mtime;

			input_path.remote = false;

			str_entry = str_entry.substr(dir_path.size()+1, str_entry.size());

			auto itr = find_in_tree(content, str_entry);
			if(itr != content.end())
				itr->input_paths.push_back(input_path);
			else
				content.insert(path(str_entry, input_path));
		}
	}catch(const std::exception& e){
		llog::warn("some files/directories were missed\n");
		llog::warn(e.what());
		exit(1);
	}
	return 0;
}
