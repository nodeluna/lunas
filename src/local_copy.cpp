#include <string>
#include "log.h"
#include "file_types.h"
#include "base.h"
#include "cppfs.h"
#include "utimes.h"



void match_mtime(const std::string& src, const std::string& dest){
	struct time_val time_val = utime::get_local(src, 3);
	utime::set_local(dest, time_val);
}


namespace fs_local {
	void copy(const std::string& src, const std::string& dest, const short& type){
		std::string count = "(" + std::to_string(base::syncing_counter) + std::string("/") + std::to_string(base::to_be_synced) + ")";
		if(type == DIRECTORY)
			llog::print(count + " [Dir] '" + dest + "'");
		else
			llog::print(count + " [File] '" + dest + "'");

		std::error_code ec;

		if(type == REGULAR_FILE){
			cppfs::copy(src, dest, ec);
			if(llog::ec(dest, ec, "couldn't sync", NO_EXIT) == false)
				return;
		}else if(type == DIRECTORY){
			cppfs::mkdir(dest, ec);
			if(llog::ec(dest, ec, "couldn't sync", NO_EXIT) == false)
				return;
		}else if(type == SYMLINK){
			cppfs::copy(src, dest, ec);
			if(llog::ec(dest, ec, "couldn't sync", NO_EXIT) == false)
				return;
		}else{
			llog::error("can't copy special file '" + src + "'");
			return;
		}

		match_mtime(src, dest);
		base::syncing_counter++;
	}
}
