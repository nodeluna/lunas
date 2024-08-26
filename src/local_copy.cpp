#include <string>
#include <filesystem>
#include "copy.h"
#include "local_copy.h"
#include "log.h"
#include "file_types.h"
#include "base.h"
#include "cppfs.h"
#include "local_attrs.h"
#include "permissions.h"


namespace fs_local {
	syncstat copy(const std::string& src, const std::string& dest, const short& type){
		std::string count = "(" + std::to_string(base::syncing_counter) + std::string("/") + std::to_string(base::to_be_synced) + ")";
		if(type == DIRECTORY)
			llog::print(count + " [Dir]  '" + dest + "'");
		else
			llog::print(count + " [File] '" + dest + "'");

		std::error_code ec;
		struct syncstat syncstat;

		if(type == REGULAR_FILE){
			cppfs::copy(src, dest, ec);
			if(llog::ec(dest, ec, "couldn't sync", NO_EXIT) == false)
				return syncstat;
			syncstat.copied_size = std::filesystem::file_size(src);
		}else if(type == DIRECTORY){
			cppfs::mkdir(dest, ec);
			if(llog::ec(dest, ec, "couldn't sync", NO_EXIT) == false)
				return syncstat;
		}else if(type == SYMLINK){
			cppfs::copy(src, dest, ec);
			if(llog::ec(dest, ec, "couldn't sync", NO_EXIT) == false)
				return syncstat;
		}else{
			llog::error("can't copy special file '" + src + "'");
			return syncstat;
		}

		if(options::dry_run == false){
			local_attrs::sync_utimes(src, dest);
			std::filesystem::perms perms = permissions::get_local(src, ec);
			llog::ec(src, ec, "couldn't get file permissions", NO_EXIT);
			ec = permissions::set_local(dest, perms);
			llog::ec(dest, ec, "couldn't set file permissions", NO_EXIT);
		}
		syncstat.code = 1;
		return syncstat;
	}
}
