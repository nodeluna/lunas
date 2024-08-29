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
	syncstat copy(const std::string& src, const std::string& dest, const short& type){
		llog::print_sync(src, dest, type);

		struct syncstat	syncstat = local_to_local::copy(src, dest, type);

		if(options::dry_run == false && syncstat.code == 1)
			local_attrs::sync_utimes(src, dest);

		syncstat.code = 1;
		return syncstat;
	}
}
