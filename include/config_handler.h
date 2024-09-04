#ifndef CONFIG_HANDLER
#define CONFIG_HANDLER

#include <string>

namespace config_filler {
	void exclude(const std::string& data);

	int mkdir(const std::string& data);

	int progress(const std::string& data);

	int dry_run(const std::string& data);

	int quiet(const std::string& data);

	int verbose(const std::string& data);

	int follow_symlink(const std::string& data);

	int resume(const std::string& data);

	int compression(const std::string& data);

	void compression_level(const std::string& data);

	int fsync(const std::string& data);

	int update(const std::string& data);

	int rollback(const std::string& data);
}

namespace lunas_info {
	void author(void);
	
	void version(void);

	void license(void);

	void help(void);
}

#endif // CONFIG_HANDLER
