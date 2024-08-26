#include <string>
#include <cstring>
#include <cerrno>
#include "local_attrs.h"
#include "log.h"
#include "utimes.h"

namespace local_attrs {
	int sync_utimes(const std::string& src, const std::string& dest){
		struct time_val time_val = utime::get_local(src, UTIMES);
		if(time_val.mtime == -1){
			llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
			return -1;
		}

		int rv = utime::set_local(dest, time_val);
		if(rv != 0){
			llog::error("couldn't sync utimes of '" + dest + "', " + std::strerror(errno));
			return -1;
		}
		return 0;
	}
}
