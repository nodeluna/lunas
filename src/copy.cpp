#include <string>
#include "copy.h"
#include "local_copy.h"
#include "remote_copy.h"


namespace lunas{
#ifdef REMOTE_ENABLED
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp, const short& type){
#else
	syncstat copy(const std::string& src, const std::string& dest, const short& type){
#endif 
#ifdef REMOTE_ENABLED
		if(src_sftp != nullptr || dest_sftp != nullptr)
			return fs_remote::copy(src, dest, src_sftp, dest_sftp, type);
		else
#endif 
			return fs_local::copy(src, dest, type);
	}
}
