#include <string>
#include <filesystem>
#include "copy.h"
#include "local_copy.h"
#include "local_to_local.h"
#include "log.h"
#include "file_types.h"
#include "base.h"
#include "cppfs.h"
#include "local_attrs.h"
#include "permissions.h"

namespace fs_local {
	syncstat copy(const std::string& src, const std::string& dest, const short& type) {
		std::string original_dest = lunas::original_dest(dest);
		llog::print_sync(src, original_dest, type);

		struct syncstat syncstat = local_to_local::copy(src, dest, type);

		{
			struct fs_local::original_name _(dest, original_dest, syncstat.code);
		}

		if (options::dry_run == false && syncstat.code == 1)
			local_attrs::sync_utimes(src, original_dest);

		return syncstat;
	}

	original_name::original_name(const std::string& dest_lspart, const std::string& original_name, int& code)
	    : lspart(dest_lspart), dest(original_name), synccode(code) {
	}

	original_name::~original_name() {
		if (options::dry_run)
			return;
		if (synccode != 1 || dest == lspart)
			return;

		std::error_code ec;
		std::filesystem::rename(lspart, dest, ec);
		if (llog::ec(lspart, ec, "couldn't rename file to its original name", NO_EXIT) == false)
			synccode = 0;
	}
}
