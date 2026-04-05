module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <unordered_map>
#	include <functional>
#	include <expected>
#	include <variant>
#endif

export module lunas.config.options.functions;
import lunas.config.filler;
import lunas.ipath;
import lunas.config.options;
import lunas.error;

export namespace lunas
{
	namespace config
	{
		using onoff_func = std::function<std::expected<std::monostate, lunas::error>(std::string, lunas::config::options&)>;

		std::unordered_map<std::string, onoff_func> get_onoff_options()
		{
			// clang-format off
			std::unordered_map<std::string, onoff_func> onoff_options = {
				{"-R",			config::filler::resume		},
				{"-resume",		config::filler::resume		},
				{"-mkdir",		config::filler::mkdir		},
				{"-make-directory",	config::filler::mkdir		},
				{"-q",			config::filler::quiet		},
				{"-quiet",		config::filler::quiet		},
				{"-v",			config::filler::verbose		},
				{"-verbose",		config::filler::verbose		},
				{"-P",			config::filler::progress	},
				{"-progress",		config::filler::progress	},
				{"-dr",			config::filler::dry_run		},
				{"-dry-run",		config::filler::dry_run		},
				{"-u",			config::filler::update		},
				{"-update",		config::filler::update		},
				{"-rb",			config::filler::rollback	},
				{"-rollback",		config::filler::rollback	},
				{"-L",			config::filler::follow_symlink	},
				{"-dereference",	config::filler::follow_symlink	},
				{"-B",			config::filler::no_broken_link	},
				{"-no-broken-link",	config::filler::no_broken_link	},
				{"-F",			config::filler::fsync		},
				{"-fsync",		config::filler::fsync		},
				{"-remove-extra",	config::filler::remove_extra	},
				{"-rmx",		config::filler::remove_extra	},
				{"-remove-before",	config::filler::remove_before	},
				{"-rmb",		config::filler::remove_before	},
				{"-remove-partials",	config::filler::remove_partials	},
				{"-rmp",		config::filler::remove_partials	},
				{"-C",			config::filler::compression	},
				{"-compression",	config::filler::compression	},
				{"-link",		config::filler::link		},
				{"-l",			config::filler::link		},
				{"-no-recursive",	config::filler::no_recursive	},
				{"-nr",			config::filler::no_recursive	},
			};

			// clang-format on 
			return onoff_options;
		}

		std::unordered_map<std::string, std::function<struct lunas::ipath::local_path(std::string)>> get_lpaths_options() {
			// clang-format off 
			std::unordered_map<std::string, std::function<struct lunas::ipath::local_path(std::string)>> lpaths_options = {
				{"-path",		config::filler::lpath_srcdest	},
				{"-p",			config::filler::lpath_srcdest	},
				{"-source",		config::filler::lpath_src	},
				{"-src",		config::filler::lpath_src	},
				{"-s",			config::filler::lpath_src	},
				{"-destination",	config::filler::lpath_dest	},
				{"-dest",		config::filler::lpath_dest	},
				{"-d",			config::filler::lpath_dest	},
			};

			// clang-format on
			return lpaths_options;
		}

		std::unordered_map<std::string, std::function<lunas::ipath::srcdest(void)>> get_rpaths_options()
		{
			// clang-format off
			std::unordered_map<std::string, std::function<lunas::ipath::srcdest(void)>> rpaths_options = {
				{"-remote-path",	config::filler::rpath_srcdest	},
				{"-r",			config::filler::rpath_srcdest	},
				{"-remote-source",	config::filler::rpath_src	},
				{"-rsrc",		config::filler::rpath_src	},
				{"-rs",			config::filler::rpath_src	},
				{"-remote-destination",	config::filler::rpath_dest	},
				{"-rdest",		config::filler::rpath_dest	},
				{"-rd",			config::filler::rpath_dest	},
			};

			// clang-format on
			return rpaths_options;
		}

		using misc_func = std::function<std::expected<std::monostate, lunas::error>(std::string, lunas::config::options&)>;

		std::unordered_map<std::string, misc_func> get_misc_options()
		{
			// clang-format off
			std::unordered_map<std::string, misc_func> misc_options = {
				{"-compression-level",	config::filler::compression_level	},
				{"-CL",			config::filler::compression_level	},
				{"-ssh-log",		config::filler::ssh_log			},
				{"-sl",			config::filler::ssh_log			},
				{"-sftp-timeout",	config::filler::sftp_timeout		},
				{"-st",			config::filler::sftp_timeout		},
				{"-sftp-retries",	config::filler::sftp_retries		},
				{"-sr",			config::filler::sftp_retries		},
				{"-connect-timeout",	config::filler::connect_timeout		},
				{"-ct",			config::filler::connect_timeout		},
				{"-exclude",		config::filler::exclude			},
				{"-x",			config::filler::exclude			},
				{"-exclude-pattern",	config::filler::exclude_pattern		},
				{"-xp",			config::filler::exclude_pattern		},
				{"-allow",		config::filler::allow			},
				{"-a",			config::filler::allow			},
				{"-allow-pattern",	config::filler::allow_pattern		},
				{"-ap",			config::filler::allow_pattern		},
				{"-minimum-space",	config::filler::minimum_space  		},
				{"-ms",			config::filler::minimum_space  		},
				{"-max-file-size",	config::filler::max_file_size  		},
				{"-MFS",		config::filler::max_file_size  		},
				{"-min-file-size",	config::filler::min_file_size  		},
				{"-mfs",		config::filler::min_file_size  		},
				{"-attributes",		config::filler::attributes     		},
				{"-A",			config::filler::attributes     		},
				{"-prehook",		config::filler::prehook     		},
				{"-pre",		config::filler::prehook     		},
				{"-posthook",		config::filler::posthook     		},
				{"-post",		config::filler::posthook     		},
				{"-mtime-grace",	config::filler::mtime_grace		},
				{"-mg",			config::filler::mtime_grace		},
			};

			// clang-format on
			return misc_options;
		}

		std::unordered_map<std::string, std::function<void(void)>> get_info_options()
		{
			// clang-format off
			std::unordered_map<std::string, std::function<void(void)>> info = {
				{"-h",			lunas::config::info::help		},
				{"-help",		lunas::config::info::help		},
				{"-author",		lunas::config::info::author		},
				{"-V",			lunas::config::info::version		},
				{"-version",		lunas::config::info::version		},
				{"-license",		lunas::config::info::license		},
				{"-meow",		lunas::config::info::meow		},
			};

			// clang-format on
			return info;
		}
	}
}
