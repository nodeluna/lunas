#ifndef CONFIG_MANAGER
#define CONFIG_MANAGER

#include <string>
#include <unordered_map>
#include <functional>
#include "config_handler.h"

#define DEMO_CONFIG "global{\n"\
	"\tmkdir = on\n"\
	"\t#compression = on\n"\
	"\t#resume = on\n"\
	"\t#progress = on\n"\
	"\t#update = on\n"\
	"}\n"\
	"\n"

inline std::unordered_map<std::string, std::function<int(std::string)>> onoff_options = {
	{"-R",			config_filler::resume		},
	{"--resume",		config_filler::resume		},
	{"R",			config_filler::resume		},
	{"resume",		config_filler::resume		},
	{"-mkdir",		config_filler::mkdir		},
	{"--make-directory",	config_filler::mkdir		},
	{"mkdir",		config_filler::mkdir		},
	{"make-directory",	config_filler::mkdir		},
	{"-q",			config_filler::quiet		},
	{"--quiet",		config_filler::quiet		},
	{"q",			config_filler::quiet		},
	{"quiet",		config_filler::quiet		},
	{"-v",			config_filler::verbose		},
	{"--verbose",		config_filler::verbose		},
	{"v",			config_filler::verbose		},
	{"verbose",		config_filler::verbose		},
	{"-P",			config_filler::progress		},
	{"--progress",		config_filler::progress		},
	{"P",			config_filler::progress		},
	{"progress",		config_filler::progress		},
	{"-dr",			config_filler::dry_run		},
	{"--dry-run",		config_filler::dry_run		},
	{"dr",			config_filler::dry_run		},
	{"dry-run",		config_filler::dry_run		},
	{"-u",			config_filler::update		},
	{"--update",		config_filler::update		},
	{"u",			config_filler::update		},
	{"update",		config_filler::update		},
	{"-rb",			config_filler::rollback		},
	{"--rollback",		config_filler::rollback		},
	{"rb",			config_filler::rollback		},
	{"rollback",		config_filler::rollback		},
	{"-L",			config_filler::follow_symlink	},
	{"--dereference",	config_filler::follow_symlink	},
	{"L",			config_filler::follow_symlink	},
	{"dereference",		config_filler::follow_symlink	},
	{"-F",			config_filler::fsync		},
	{"--fsync",		config_filler::fsync		},
	{"F",			config_filler::fsync		},
	{"fsync",		config_filler::fsync		},
#ifdef REMOTE_ENABLED
	{"-C",			config_filler::compression	},
	{"--compression",	config_filler::compression	},
	{"C",			config_filler::compression	},
	{"compression",		config_filler::compression	},
#endif // REMOTE_ENABLED
};

inline std::unordered_map<std::string, std::function<void(void)>> info = {
	{"-h",			lunas_info::help		},
	{"--help",		lunas_info::help		},
	{"--author",		lunas_info::author		},
	{"-V",			lunas_info::version		},
	{"--version",		lunas_info::version		},
	{"--license",		lunas_info::license		},
};

namespace config_manager {
	int make_demo_config(void);

	void preset(const std::string& name);
}

#endif // CONFIG_MANAGER
