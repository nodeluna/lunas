#include <string>
#include <vector>
#include <set>
#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED

#include "resume.h"
#include "init.h"
#include "base.h"
#include "log.h"
#include "init.h"
#include "copy.h"
#include "cppfs.h"
#include "raii_sftp.h"
#include "cliargs.h"



namespace resume {
	bool is_lspart(const std::string& path){
		if(path.size() > 8 && path.substr(path.size() - 8, path.size()) == ".ls.part")
			return true;
		return false;
	}

	size_t get_src_hash(const std::string& src, const unsigned long int& src_mtime){
		return std::hash<std::string>{}(src + std::to_string(src_mtime));
	}

	std::string get_dest_hash(const std::string& dest, const size_t& src_mtimepath_hash){
		int dot_counter = 0;
		unsigned long int i = dest.size() - 1;
		for(; ; i--){
			if(dot_counter == 3)
				break;
			if(dest.at(i) == '.')
				dot_counter++;
			if(i == 0)
				break;
		}

		return dest.substr(i + 2, std::to_string(src_mtimepath_hash).size());
	}

	void unlink(const sftp_session& sftp, const std::string& dest, const short& type){
		if(sftp != nullptr){
			int rc; 
			if(type == DIRECTORY)
				rc = sftp::rmdir(sftp, dest);
			else
				rc = sftp::unlink(sftp, dest);
			llog::rc(sftp, dest, rc, "couldn't remove orphaned file", NO_EXIT);
		}else{
			std::error_code ec = cppfs::remove(dest);
			llog::ec(dest, ec, "couldn't remove orphaned file", NO_EXIT);
		}
	}

	void sync(std::set<path>::iterator& itr, const struct path& lspart, const unsigned long int& dest_index){
		unsigned long int src_mtime_i = get_src(*itr);
		if(src_mtime_i == std::numeric_limits<unsigned long int>::max())
			return;
		const long int& src_mtime = itr->metadatas.at(src_mtime_i).mtime;
		const long int& dest_mtime = itr->metadatas.at(dest_index).mtime;

		std::string src = input_paths.at(src_mtime_i).path + itr->name;
		std::string dest = input_paths.at(dest_index).path + lspart.name;
		const short& type = lspart.metadatas.at(dest_index).type;

		if(src_mtime == dest_mtime || (type == DIRECTORY && dest_mtime != NON_EXISTENT)){
			llog::print("file already exists '" + dest + "', removing temp file");
			resume::unlink(input_paths.at(dest_index).sftp, dest, type);
			return;
		}

		size_t src_hash = resume::get_src_hash(src, src_mtime);
		size_t dest_hash = std::stoul(resume::get_dest_hash(lspart.name, src_hash));
		if(src_hash != dest_hash){
			llog::error("orphaned file '" + dest + "', its source has been modified. removing it");
			resume::unlink(input_paths.at(dest_index).sftp, dest, type);
			return;
		}
		if(type != REGULAR_FILE)
			resume::unlink(input_paths.at(dest_index).sftp, dest, type);
#ifdef REMOTE_ENABLED
		syncstat syncstat = lunas::copy(src, dest, input_paths.at(src_mtime_i).sftp, input_paths.at(dest_index).sftp, type);
#else
		struct syncstt syncstat = lunas::copy(src, dest, type);
#endif // REMOTE_ENABLED
		register_sync(syncstat, dest_index, type);
		if(syncstat.code == 1)
			itr->metadatas.at(dest_index).mtime = itr->metadatas.at(src_mtime_i).mtime;
	}

	void init(){
		llog::print(" ~~ resume enabled ~~");
		for(const auto& lspart : part_files){
			unsigned long int dest_index = 0;
			for(const auto& i : lspart.metadatas){
				if(i.type != -1)
					break;
				dest_index++;
			}

			std::string dest = lunas::original_dest(lspart.name);

			auto itr = content.find(path(dest, lspart.metadatas.at(0), 0));
			if(itr != content.end())
				resume::sync(itr, lspart, dest_index);
			else
				llog::error("couldn't find source for '" + input_paths.at(dest_index).path + lspart.name + "'");
		}
		part_files.clear();
		options::resume = false;
		llog::print(" ~~ resume finished ~~\n");
	}
}
