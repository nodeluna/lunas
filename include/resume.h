#ifndef RESUME
#define RESUME

#include <string>
#include <set>
#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>
#endif // REMOTE_ENABLED

#include "base.h"


namespace resume {
	bool is_lspart(const std::string& path);

	size_t get_src_hash(const std::string& src, const unsigned long int& src_mtime);

	std::string get_dest_hash(const std::string& dest, const size_t& src_mtimepath_hash);

#ifdef REMOTE_ENABLED
	void unlink(const sftp_session& sftp, const std::string& dest, const short& type);
#else
	void unlink(const std::string& dest);
#endif // REMOTE_ENABLED

	void sync(std::set<base::path>::iterator& itr, const struct base::path& lspart, std::string& dest, const unsigned long int& dest_index);

	void init();
}

#endif // RESUME
