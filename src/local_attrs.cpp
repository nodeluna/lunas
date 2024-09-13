#include <cstdlib>
#include <string>
#include <cstring>
#include <cerrno>
#include <filesystem>
#include <expected>
#include "local_attrs.h"
#include "log.h"
#include "utimes.h"
#include "permissions.h"
#include "ownership.h"


namespace local_attrs {
	int sync_utimes(const std::string& src, const std::string& dest){
		std::expected<struct time_val, int> time_val = utime::get_local(src, UTIMES);
		if(not time_val){
			llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
			return 0;
		}

		int rv = utime::set_local(dest, time_val.value());
		if(rv != 0){
			llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
			return 0;
		}
		return 1;
	}

	int sync_permissions(const std::string& src, const std::string& dest){
		std::error_code ec;
		std::expected<std::filesystem::perms, std::error_code> perms = permissions::get_local(src);
		if(not perms){
			llog::ec(src, ec, "couldn't get file permissions", NO_EXIT);
			return 0;
		}

		ec = permissions::set_local(dest, perms.value());
		if(llog::ec(dest, ec, "couldn't set file permissions", NO_EXIT) == false)
			return 0;
		return 1;
	}

	bool sync_ownership(const std::string& src, const std::string& dest){
		if(not options::attributes_uid && not options::attributes_gid)
			return true;

		std::expected<struct own, std::error_code> own = ownership::get_local(src);
		if(not own){
			llog::ec(src, own.error(), "couldn't get file ownership", NO_EXIT); 
			return false;
		}

		if(not options::attributes_uid)
			own.value().uid = -1;
		if(not options::attributes_gid)
			own.value().gid = -1;

		std::optional<std::error_code> err = ownership::set_local(dest, own.value());
		if(err){
			llog::ec(src, *err, "couldn't set file ownership", NO_EXIT); 
			return false;
		}

		return true;
	}
}
