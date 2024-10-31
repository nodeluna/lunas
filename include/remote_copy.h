#ifndef HANDLING_COPYING_REMOTE
#define HANDLING_COPYING_REMOTE

#include "config.h"

#ifdef REMOTE_ENABLED

#	include <string>
#	include <vector>
#	include <cstdint>
#	include <libssh/sftp.h>
#	include "copy.h"

struct buffque {
		std::vector<char> buffer;
		int		  bytes_xfered = 0;
		sftp_aio	  aio	       = NULL;

		explicit buffque(std::uint64_t size) : buffer(size) {
		}
};

namespace fs_remote {
	syncstat copy(const std::string& src, const std::string& dest, const sftp_session& src_sftp, const sftp_session& dest_sftp,
	    const short& type);

	struct original_name {
			original_name(
			    const sftp_session& session, const std::string& dest_lspart, const std::string& original_name, int& code);
			~original_name();

		private:
			const sftp_session& sftp;
			const std::string&  lspart;
			const std::string&  dest;
			int&		    synccode;
	};

}

#endif // REMOTE_ENABLED

#endif // HANDLING_COPYING_REMOTE
