#ifndef LOCAL_ATTRS
#define LOCAL_ATTRS

#include <string>

namespace local_attrs {
	int sync_utimes(const std::string& src, const std::string& dest);
	int sync_permissions(const std::string& src, const std::string& dest);
}

#endif // LOCAL_ATTRS
