#ifndef FS_REMOTE
#define FS_REMOTE

#include "config.h"

#ifdef REMOTE_ENABLED

#include <string>
#include <libssh/sftp.h>
#include "base.h"
#include "cliargs.h"
#include "raii_sftp.h"

namespace fs_remote {
	void list_tree(struct input_path& local_path, const unsigned long int& index_path);

	std::expected<std::uintmax_t, SSH_STATUS> available_space(const struct input_path& remote_path);
}

#endif // REMOTE_ENABLED

#endif // FS_REMOTE
