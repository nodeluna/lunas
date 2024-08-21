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
#include "cliargs.h"

namespace fs = std::filesystem;

int list_tree(const struct input_path& local_path, const unsigned long int& path_index){
	const std::string& dir_path = local_path.path;
	short  input_type;

	if(fs::exists(dir_path) == false && options::mkdir)
		fs::create_directory(dir_path);

	if((input_type = status::local_type(dir_path, true)) != DIRECTORY && input_type != -1){
		llog::warn("path '" + dir_path + "' not a directory");
		return 1;
	}else if(input_type == -1){
		return 1;
	}

	try {
		for(const auto& entry : fs::recursive_directory_iterator(dir_path, fs::directory_options::skip_permission_denied)){
			std::string str_entry = entry.path().string();
			struct metadata metadata;

			short type = status::local_type(str_entry, false);
			if(type == -1)
				llog::local_error(str_entry, "couldn't get type of", EXIT_FAILURE);
			metadata.type = type;

			struct time_val time_val = utime::get_local(str_entry, 2);
			if(time_val.mtime == 0)
				llog::local_error(str_entry, "couldn't get mtime of file", EXIT_FAILURE);
			metadata.mtime = time_val.mtime;

			str_entry = str_entry.substr(dir_path.size(), str_entry.size());

			auto itr = content.find(path(str_entry, metadata, path_index));
			if(itr != content.end())
				itr->metadatas[path_index] = metadata;
			else
				content.insert(path(str_entry, metadata, path_index));
		}
	}catch(const std::exception& e){
		llog::warn("some files/directories were missed\n");
		llog::warn(e.what());
		exit(1);
	}
	return 0;
}
