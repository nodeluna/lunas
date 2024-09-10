#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <variant>
#include <cmath>
#include "config.h"
#include "file_types.h"
#include "log.h"
#include "base.h"
#include "raii_sftp.h"

namespace fs = std::filesystem;


std::string get_type_name(const short& type){
	if(type == REGULAR_FILE)
		return "regular file";
	else if(type == DIRECTORY)
		return "directory";
	else if(type == SYMLINK)
		return "symlink";
	else if(type == BROKEN_SYMLINK)
		return "broken symlink";
	else
		return "special file";
}

namespace status{
	unsigned short int local_types(fs::file_status entry){
		unsigned short int type;
		if(entry.type() == fs::file_type::symlink){
			type = SYMLINK;
		}else if(entry.type() == fs::file_type::directory){
			type = DIRECTORY;
		}else if(entry.type() == fs::file_type::regular){
			type = REGULAR_FILE;
		}else if(entry.type() == fs::file_type::socket){
			type = SOCKET;
		}else{
			type = SPECIAL_TYPE;
		}
		return type;
	}

	short int local_type(const std::string& path, const bool& cerr){
		std::error_code ec;
		fs::file_status status;
		if(options::follow_symlink == true)
			status = fs::status(path, ec);
		else
			status = fs::symlink_status(path, ec);
		
		if(ec.value() != 0){
			if(cerr)
				llog::local_error(path, "couldn't get type of", NO_EXIT);
			return -1;
		}
		return local_types(status);
	}

	std::variant<bool, std::error_code> is_broken_link(const std::string& path){
		std::error_code ec;
		if(not fs::is_symlink(path, ec))
			return false;
		if(ec.value() != 0)
			return ec;

		std::string target = fs::read_symlink(path, ec);
		if(ec.value() != 0)
			return ec;

		bool exists = fs::exists(target, ec);
		if(ec.value() != 0)
			return ec;

		return not exists;
	}

#ifdef REMOTE_ENABLED
	short int remote_type(const sftp_attributes& attributes){
		if(attributes == NULL)
			return -1;
		if(attributes->type == SSH_FILEXFER_TYPE_SYMLINK)
			return SYMLINK;
		else if(attributes->type == SSH_FILEXFER_TYPE_DIRECTORY)
			return DIRECTORY;
		else if(attributes->type == SSH_FILEXFER_TYPE_REGULAR)
			return REGULAR_FILE;
		else
			return SPECIAL_TYPE;
    }

	short int remote_type2(const sftp_session& sftp, const std::string& path, bool cerr){
		sftp_attributes attributes;
		if(options::follow_symlink)
			attributes = sftp_stat(sftp, path.c_str());
		else
			attributes = sftp_lstat(sftp, path.c_str());
		if(attributes == NULL){
			if(cerr)
				llog::remote_error(sftp, path, "couldn't get type of", NO_EXIT);
			return -1;
		}
		raii::sftp::attributes attr_obj = raii::sftp::attributes(&attributes);
		return remote_type(attributes);
	}
#endif // REMOTE_ENABLED
}


namespace condition{
	bool is_src(const short& type){
		if(type == SRC || type == SRCDEST)
			return true;
		return false;
	}

	bool is_dest(const short& type){
		if(type == DEST || type == SRCDEST)
			return true;
		return false;
	}
}
