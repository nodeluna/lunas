#ifndef CONFIG_MANAGER
#define CONFIG_MANAGER

#include <string>
#include <unordered_map>
#include <functional>
#include <optional>
#include "config_handler.h"

#define DEMO_CONFIG                 \
	"global{\n"                 \
	"\t#mkdir = on\n"           \
	"\t#compression = on\n"     \
	"\t#resume = on\n"          \
	"\t#progress = on\n"        \
	"\t#update = on\n"          \
	"\t#minimum-space = 1gib\n" \
	"\t#attributes = mtime\n"   \
	"}\n"                       \
	"luna {\n"                  \
	"\tpath = /path/to/dir1\n"  \
	"\tpath = /path/to/dir2\n"  \
	"\tpath = /path/to/dir3\n"  \
	"\tremote {\n"              \
	"\t\tr = user@ip:dir\n"     \
	"\t\tport = 22\n"           \
	"\t}\n"                     \
	"}\n"

// clang-format off

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
	{"-B",			config_filler::no_broken_link	},
	{"--no-broken-link",	config_filler::no_broken_link	},
	{"B",			config_filler::no_broken_link	},
	{"no-broken-link",	config_filler::no_broken_link	},
	{"-F",			config_filler::fsync		},
	{"--fsync",		config_filler::fsync		},
	{"F",			config_filler::fsync		},
	{"fsync",		config_filler::fsync		},
	{"--remove-extra",	config_filler::remove_extra	},
	{"-rmx",		config_filler::remove_extra	},
	{"remove-extra",	config_filler::remove_extra	},
	{"rmx",			config_filler::remove_extra	},
#ifdef REMOTE_ENABLED
	{"-C",			config_filler::compression	},
	{"--compression",	config_filler::compression	},
	{"C",			config_filler::compression	},
	{"compression",		config_filler::compression	},
#endif // REMOTE_ENABLED
};

inline std::unordered_map<std::string, std::function<int(std::string)>> lpaths_options = {
	{"--path",		config_filler::lpath_srcdest	},
	{"-p",			config_filler::lpath_srcdest	},
	{"path",		config_filler::lpath_srcdest	},
	{"p",			config_filler::lpath_srcdest	},
	{"--source",		config_filler::lpath_src	},
	{"-src",		config_filler::lpath_src	},
	{"-s",			config_filler::lpath_src	},
	{"source",		config_filler::lpath_src	},
	{"src",			config_filler::lpath_src	},
	{"s",			config_filler::lpath_src	},
	{"--destination",	config_filler::lpath_dest	},
	{"-dest",		config_filler::lpath_dest	},
	{"-d",			config_filler::lpath_dest	},
	{"destination",		config_filler::lpath_dest	},
	{"dest",		config_filler::lpath_dest	},
	{"d",			config_filler::lpath_dest	},
};

inline std::unordered_map<std::string, std::function<short(void)>> rpaths_options = {
	{"--remote-path",	config_filler::rpath_srcdest	},
	{"-r",			config_filler::rpath_srcdest	},
	{"remote-path",		config_filler::rpath_srcdest	},
	{"r",			config_filler::rpath_srcdest	},
	{"--remote-source",	config_filler::rpath_src	},
	{"-rsrc",		config_filler::rpath_src	},
	{"-rs",			config_filler::rpath_src	},
	{"remote-source",	config_filler::rpath_src	},
	{"rsrc",		config_filler::rpath_src	},
	{"rs",			config_filler::rpath_src	},
	{"--remote-destination",config_filler::rpath_dest	},
	{"-rdest",		config_filler::rpath_dest	},
	{"-rd",			config_filler::rpath_dest	},
	{"remote-destination",	config_filler::rpath_dest	},
	{"rdest",		config_filler::rpath_dest	},
	{"rd",			config_filler::rpath_dest	},
};

inline std::unordered_map<std::string, std::function<int(std::string)>> misc_options = {
	{"--compression-level",	config_filler::compression_level},
	{"-CL",			config_filler::compression_level},
	{"compression-level",	config_filler::compression_level},
	{"CL",			config_filler::compression_level},
	{"--exclude",		config_filler::exclude		},
	{"-x",			config_filler::exclude		},
	{"exclude",		config_filler::exclude		},
	{"x",			config_filler::exclude		},
	{"--exclude-pattern",	config_filler::exclude_pattern	},
	{"-xp",			config_filler::exclude_pattern	},
	{"exclude-pattern",	config_filler::exclude_pattern	},
	{"xp",			config_filler::exclude_pattern	},
	{"--minimum-space",	config_filler::minimum_space	},
	{"-ms",			config_filler::minimum_space	},
	{"minimum-space",	config_filler::minimum_space	},
	{"ms",			config_filler::minimum_space	},
	{"--attributes",	config_filler::attributes	},
	{"-a",			config_filler::attributes	},
	{"attributes",		config_filler::attributes	},
	{"a",			config_filler::attributes	},
};

inline std::unordered_map<std::string, std::function<void(void)>> info = {
	{"-h",			lunas_info::help		},
	{"--help",		lunas_info::help		},
	{"--author",		lunas_info::author		},
	{"-V",			lunas_info::version		},
	{"--version",		lunas_info::version		},
	{"--license",		lunas_info::license		},
	{"meow",		lunas_info::meow		},
	{"-meow",		lunas_info::meow		},
	{"--meow",		lunas_info::meow		},
};

// clang-format on

namespace config_manager {
	inline std::string config_dir  = std::getenv("HOME") + std::string("/.config/lunas/");
	inline std::string file_name   = std::string("lunas.luco");
	inline std::string config_path = config_dir + file_name;

	std::optional<std::string> make_demo_config(void);

	std::optional<std::string> preset(const std::string& name);
}

#endif // CONFIG_MANAGER
