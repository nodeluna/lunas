#ifndef CONFIG_HANDLER
#define CONFIG_HANDLER

#include <string>

namespace config_filler {
	int lpath_srcdest(const std::string& data);

	int lpath_src(const std::string& data);

	int lpath_dest(const std::string& data);

	short rpath_srcdest(void);

	short rpath_src(void);

	short rpath_dest(void);

	int exclude(const std::string& data);

	int exclude_pattern(const std::string& data);

	int mkdir(const std::string& data);

	int progress(const std::string& data);

	int dry_run(const std::string& data);

	int quiet(const std::string& data);

	int verbose(const std::string& data);

	int follow_symlink(const std::string& data);

	int no_broken_link(const std::string& data);

	int resume(const std::string& data);

	int compression(const std::string& data);

	int compression_level(const std::string& data);

	int minimum_space(const std::string& data);

	int attributes(const std::string& data);

	int fsync(const std::string& data);

	int remove_extra(const std::string& data);

	int update(const std::string& data);

	int rollback(const std::string& data);
}

namespace lunas_info {
	void author(void);
	
	void version(void);

	void license(void);

	void help(void);

	void meow(void);
}

#endif // CONFIG_HANDLER
