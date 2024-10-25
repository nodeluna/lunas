#include <cerrno>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <string>
#include <iostream>
#include <set>
#include <expected>
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
#include "path_parsing.h"


namespace fs = std::filesystem;

namespace local_readdir_operations {
	void type(struct metadata& metadata, const std::string& str_entry);

	void mtime(struct metadata& metadata, const std::string& str_entry);

	bool lspart(const struct input_path& local_path, const struct metadata& metadata, const std::string& str_entry, const size_t& path_index);
	
	void insert(const struct metadata& metadata, const std::string& str_entry, const size_t& path_index);
}

namespace fs_local {
	int readdir(const struct input_path& local_path, const std::string& dir_path, const unsigned long int& path_index){
		try {
			for(const auto& entry : fs::directory_iterator(dir_path)){
				std::string str_entry = entry.path().string();
				if(utils::exclude(str_entry, local_path.path))
						continue;
				struct metadata metadata;

				local_readdir_operations::type(metadata, str_entry);
				local_readdir_operations::mtime(metadata, str_entry);

				if(local_readdir_operations::lspart(local_path, metadata, str_entry, path_index))
					continue;

				str_entry = str_entry.substr(local_path.path.size(), str_entry.size());

				local_readdir_operations::insert(metadata, str_entry, path_index);

				if(metadata.type == DIRECTORY)
					readdir(local_path, entry.path().string(), path_index);
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

		return fs_local::readdir(local_path, local_path.path, path_index);
	}

	std::expected<std::uintmax_t, std::error_code> available_space(struct input_path& local_path){
		std::error_code ec;
		if(fs::exists(local_path.path, ec) == false){
			if(fs::exists(parse_path::get_lower_dir_level(local_path.path), ec) == false){
				ec = std::make_error_code(std::errc::no_such_file_or_directory);
				return std::unexpected(ec);
			}
			if(ec.value() != 0)
				return std::unexpected(ec);
			fs::space_info availabe_space = fs::space(parse_path::get_lower_dir_level(local_path.path), ec);
			if(ec.value() != 0)
				return std::unexpected(ec);
			return availabe_space.available;
		}
		if(ec.value() != 0)
			return std::unexpected(ec);
		fs::space_info availabe_space = fs::space(local_path.path, ec);
		if(ec.value() != 0)
			return std::unexpected(ec);
		return availabe_space.available;
	}
}

namespace local_readdir_operations {
	void type(struct metadata& metadata, const std::string& str_entry){
		if(options::no_broken_symlink){
			std::expected<bool, std::error_code> broken = status::is_broken_link(str_entry);
			if(not broken.has_value())
				llog::local_error(str_entry, "couldn't check link", EXIT_FAILURE);
			else if(broken.value()){
				metadata.type = BROKEN_SYMLINK;
				return;
			}
		}

		metadata.type = status::local_type(str_entry, false);
		if(metadata.type == -1)
			llog::local_error(str_entry, "couldn't get type of", EXIT_FAILURE);
	}

	void mtime(struct metadata& metadata, const std::string& str_entry){
		if(options::no_broken_symlink && metadata.type == BROKEN_SYMLINK){
			std::expected<struct time_val, int> time_val = utime::lget_local(str_entry, 2);
			if(not time_val)
				llog::local_error(str_entry, "couldn't get mtime of file", EXIT_FAILURE);
			metadata.mtime = time_val.value().mtime;
		}else{
			std::expected<struct time_val, int> time_val = utime::get_local(str_entry, 2);
			if(not time_val)
				llog::local_error(str_entry, "couldn't get mtime of file", EXIT_FAILURE);
			metadata.mtime = time_val.value().mtime;
		}

	}

	bool lspart(const struct input_path& local_path, const struct metadata& metadata, const std::string& str_entry, const size_t& path_index){
		if(resume::is_lspart(str_entry)){
			if(options::resume){
				std::string relative_path = str_entry.substr(local_path.path.size(), str_entry.size());
				part_files.insert(base::path(relative_path, metadata, path_index));
			}else{
				std::error_code ec = cppfs::remove(str_entry);
				llog::ec(str_entry, ec, "couldn't remove incomplete file.ls.part", NO_EXIT); 
			}
			return true;
		}
		
		return false;
	}
	
	void insert(const struct metadata& metadata, const std::string& str_entry, const size_t& path_index){
		auto itr = content.find(base::path(str_entry, metadata, path_index));
		if(itr != content.end())
			itr->metadatas[path_index] = metadata;
		else
			content.insert(base::path(str_entry, metadata, path_index));
	}
}
