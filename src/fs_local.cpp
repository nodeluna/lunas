#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>
#include <iostream>
#include <set>
#include "base.h"
#include "fs_local.h"
#include "file_types.h"
#include "utimes.h"
#include "log.h"
#include "cliargs.h"
#include "cppfs.h"
#include "resume.h"
#include "exclude.h"
#include "permissions.h"

namespace fs = std::filesystem;

namespace fs_local {
	int readdir(const struct input_path& local_path, const unsigned long int& path_index){
		try {
			for(const auto& entry : fs::recursive_directory_iterator(local_path.path, fs::directory_options::skip_permission_denied)){
				std::string str_entry = entry.path().string();
				if(utils::exclude(str_entry, local_path.path))
						continue;
				struct metadata metadata;
				short type = status::local_type(str_entry, false);
				if(type == -1)
					llog::local_error(str_entry, "couldn't get type of", EXIT_FAILURE);
				metadata.type = type;

				struct time_val time_val = utime::get_local(str_entry, 2);
				if(time_val.mtime == -1)
					llog::local_error(str_entry, "couldn't get mtime of file", EXIT_FAILURE);
				metadata.mtime = time_val.mtime;

				if(resume::is_lspart(str_entry)){
					if(options::resume){
						std::string relative_path = str_entry.substr(local_path.path.size(), str_entry.size());
						part_files.insert(path(relative_path, metadata, path_index));
					}else{
						std::error_code ec = cppfs::remove(str_entry);
						llog::ec(str_entry, ec, "couldn't remove incomplete file.ls.part", NO_EXIT); 
					}
					continue;
				}

				str_entry = str_entry.substr(local_path.path.size(), str_entry.size());

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

	int list_tree(struct input_path& local_path, const unsigned long int& path_index){
		short input_type;
		std::error_code ec;
		if(fs::exists(local_path.path, ec) == false && options::mkdir){
			cppfs::mkdir(local_path.path, ec);
			llog::ec(local_path.path, ec, "couldn't create input directory", EXIT_FAILURE);
			llog::print("-[!] created input directory '" + local_path.path + "', it was not found");
			os::append_seperator(local_path.path);
			if(options::dry_run)
				return 0;
		}
		llog::ec(local_path.path, ec, "couldn't check input directory", EXIT_FAILURE);

		fs::file_status status = fs::status(local_path.path, ec);
		llog::ec(local_path.path, ec, "couldn't check input path", EXIT_FAILURE);
		if((input_type = status::local_types(status) != DIRECTORY) && input_type != -1){
			llog::error("input path '" + local_path.path + "' isn't a directory");
			exit(1);
		}else if(input_type == -1)
			exit(1);

		if(permissions::is_local_readable(local_path.path) == false){
			llog::error("couldn't read input directory '" + local_path.path + "', permission denied");
			exit(1);
		}

		return fs_local::readdir(local_path, path_index);
	}
}
