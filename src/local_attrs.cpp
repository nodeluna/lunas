#include <cstdlib>
#include <string>
#include <cstring>
#include <cerrno>
#include <filesystem>
#include <variant>
#include "local_attrs.h"
#include "log.h"
#include "utimes.h"
#include "permissions.h"

namespace local_attrs {
	int sync_utimes(const std::string& src, const std::string& dest){
		std::variant<struct time_val, int> time_val = utime::get_local(src, UTIMES);
		if(std::holds_alternative<int>(time_val)){
			llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
			return 0;
		}

		int rv = utime::set_local(dest, std::get<struct time_val>(time_val));
		if(rv != 0){
			llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
			return 0;
		}
		return 1;
	}

	int sync_permissions(const std::string& src, const std::string& dest){
		std::error_code ec;
		std::filesystem::perms perms = permissions::get_local(src, ec);
		if(llog::ec(src, ec, "couldn't get file permissions", NO_EXIT) == false)
			return 0;
		ec = permissions::set_local(dest, perms);
		if(llog::ec(dest, ec, "couldn't set file permissions", NO_EXIT) == false)
			return 0;
		return 1;
	}
}
