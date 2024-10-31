#include <string>
#include "copy.h"
#include "local_copy.h"
#include "remote_copy.h"
#include "resume.h"

namespace lunas {
#ifdef REMOTE_ENABLED
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp,
	    const short& type) {
#else
	syncstat copy(const std::string& src, const std::string& dest, const short& type) {
#endif
#ifdef REMOTE_ENABLED
		if (src_sftp != nullptr || dest_sftp != nullptr)
			return fs_remote::copy(src, dest, src_sftp, dest_sftp, type);
		else
#endif
			return fs_local::copy(src, dest, type);
	}

	std::string original_dest(const std::string& dest_with_hash) {
		if (dest_with_hash.empty() || resume::is_lspart(dest_with_hash) == false)
			return dest_with_hash;

		std::string	  dest;
		int		  dot_counter = 0;
		unsigned long int i	      = dest_with_hash.size() - 1;
		for (;; i--) {
			if (dot_counter == 3)
				break;
			if (dest_with_hash.at(i) == '.')
				dot_counter++;
			if (i == 0)
				break;
		}
		dest = dest_with_hash.substr(0, i + 1);
		return dest;
	}
}
