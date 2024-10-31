#include <cctype>
#include <functional>
#include <string>
#include <iostream>
#include <unordered_map>
#include "config_handler.h"
#include "config.h"
#include "cliargs.h"
#include "log.h"
#include "about.h"
#include "base.h"
#include "path_parsing.h"
#include "os.h"

namespace config_filler {
	void fill_local_path(const std::string& argument, const short& srcdest) {
		struct input_path local_path;
		local_path.path	   = parse_path::absolute(argument);
		local_path.srcdest = srcdest;
		input_paths.push_back(std::move(local_path));
	}

	int lpath_srcdest(const std::string& data) {
		config_filler::fill_local_path(data, SRCDEST);
		return 0;
	}

	int lpath_src(const std::string& data) {
		config_filler::fill_local_path(data, SRC);
		return 0;
	}

	int lpath_dest(const std::string& data) {
		config_filler::fill_local_path(data, DEST);
		return 0;
	}

	short rpath_srcdest(void) {
		return SRCDEST;
	}

	short rpath_src(void) {
		return SRC;
	}

	short rpath_dest(void) {
		return DEST;
	}

	int compression_level(const std::string& data) {
		if (is_num(data) == false)
			llog::error_exit("argument '" + data + "' for option compression-level isn't a number", EXIT_FAILURE);

		int level = std::stoi(data);
		if (level > 9 || level <= 0)
			llog::error_exit("compression level must be between 1-9. provided level '" + data + "'", EXIT_FAILURE);
		options::compression = level;
		return 0;
	}

	int minimum_space(const std::string& data) {
		constexpr std::uintmax_t KiB  = 1024;
		constexpr std::uintmax_t MiB  = 1024 * KiB;
		constexpr std::uintmax_t GiB  = 1024 * MiB;
		constexpr std::uintmax_t TiB  = 1024 * GiB;
		constexpr std::uintmax_t PiB  = 1024 * TiB;
		std::string		 temp = data;

		std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c) { return std::tolower(c); });
		if (temp.find("kib") != data.npos)
			temp.resize(temp.find("kib"));
		else if (temp.find("mib") != temp.npos)
			temp.resize(temp.find("mib"));
		else if (temp.find("gib") != temp.npos)
			temp.resize(temp.find("gib"));
		else if (temp.find("tib") != temp.npos)
			temp.resize(temp.find("tib"));
		else if (temp.find("pib") != temp.npos)
			temp.resize(temp.find("pib"));
		else
			temp = data;

		if (is_num_decimal(temp) == false)
			llog::error_exit("argument '" + data + "' for option minimum-space isn't valid", EXIT_FAILURE);

		double	       num	     = std::stod(data);
		std::uintmax_t minimum_space = 0;

		temp = data;
		std::transform(temp.begin(), temp.end(), temp.begin(), [](unsigned char c) { return std::tolower(c); });

		if (temp.find("kib") != data.npos)
			minimum_space = num * KiB;
		else if (temp.find("mib") != temp.npos)
			minimum_space = num * MiB;
		else if (temp.find("gib") != temp.npos)
			minimum_space = num * GiB;
		else if (temp.find("tib") != temp.npos)
			minimum_space = num * TiB;
		else if (temp.find("pib") != temp.npos)
			minimum_space = num * PiB;
		else
			minimum_space = num;

		options::minimum_space = minimum_space;
		return 0;
	}

	int exclude(const std::string& data) {
		std::string path = data;
		os::pop_seperator(path);
		options::exclude.insert(path);
		return 0;
	}

	int exclude_pattern(const std::string& data) {
		std::string path = data;
		os::pop_seperator(path);
		options::exclude_pattern.insert(path);
		return 0;
	}

	int mkdir(const std::string& data) {
		if (data == "on")
			options::mkdir = true;
		else if (data == "off")
			options::mkdir = false;
		else
			return -1;
		return 0;
	}

	int progress(const std::string& data) {
		if (data == "on") {
			options::progress_bar = true;
		} else if (data == "off") {
			options::progress_bar = false;
		} else
			return -1;
		return 0;
	}

	int dry_run(const std::string& data) {
		if (data == "on") {
			llog::print("--> dry run is on\n");
			options::dry_run = true;
		} else if (data == "off")
			options::dry_run = false;
		else
			return -1;
		return 0;
	}

	int quiet(const std::string& data) {
		if (data == "on")
			options::quiet = true;
		else if (data == "off")
			options::quiet = false;
		else
			return -1;
		return 0;
	}

	int verbose(const std::string& data) {
		if (data == "on")
			options::verbose = true;
		else if (data == "off")
			options::verbose = false;
		else
			return -1;
		return 0;
	}

	int follow_symlink(const std::string& data) {
		if (data == "on")
			options::follow_symlink = true;
		else if (data == "off")
			options::follow_symlink = false;
		else
			return -1;
		return 0;
	}

	int no_broken_link(const std::string& data) {
		if (data == "on")
			options::no_broken_symlink = true;
		else if (data == "off")
			options::no_broken_symlink = false;
		else
			return -1;
		return 0;
	}

	int resume(const std::string& data) {
		if (data == "on")
			options::resume = true;
		else if (data == "off")
			options::resume = false;
		else
			return -1;
		return 0;
	}

	int compression(const std::string& data) {
		if (data == "on")
			options::compression = true;
		else if (data == "off")
			options::compression = false;
		else
			return -1;
		return 0;
	}

	int attributes_uid(const std::string& data) {
		if (data == "on")
			options::attributes_uid = true;
		else if (data == "off")
			options::attributes_uid = false;
		else
			return -1;
		return 0;
	}

	int attributes_gid(const std::string& data) {
		if (data == "on")
			options::attributes_gid = true;
		else if (data == "off")
			options::attributes_gid = false;
		else
			return -1;
		return 0;
	}

	int attributes_own(const std::string& data) {
		if (data == "on") {
			options::attributes_uid = true;
			options::attributes_gid = true;
		} else if (data == "off") {
			options::attributes_uid = false;
			options::attributes_gid = false;
		} else
			return -1;
		return 0;
	}

	int attributes_atime(const std::string& data) {
		if (data == "on")
			options::attributes_atime = true;
		else if (data == "off")
			options::attributes_atime = false;
		else
			return -1;
		return 0;
	}

	int attributes_mtime(const std::string& data) {
		if (data == "on")
			options::attributes_mtime = true;
		else if (data == "off")
			options::attributes_mtime = false;
		else
			return -1;
		return 0;
	}

	int attributes_utimes(const std::string& data) {
		if (data == "on") {
			options::attributes_atime = true;
			options::attributes_mtime = true;
		} else if (data == "off") {
			options::attributes_atime = false;
			options::attributes_mtime = false;
		} else
			return -1;
		return 0;
	}

	int attributes_all(const std::string& data) {
		if (data == "on") {
			options::attributes_atime = true;
			options::attributes_mtime = true;
			options::attributes_uid	  = true;
			options::attributes_gid	  = true;
		} else if (data == "off") {
			options::attributes_atime = false;
			options::attributes_mtime = false;
			options::attributes_uid	  = false;
			options::attributes_gid	  = false;
		} else
			return -1;
		return 0;
	}

	int attributes(const std::string& data) {
		std::string								temp;
		std::unordered_map<std::string, std::function<int(const std::string&)>> attributes_options = {
		    {   "own",    config_filler::attributes_own},
		    {   "uid",    config_filler::attributes_uid},
		    {   "gid",    config_filler::attributes_gid},
		    { "atime",  config_filler::attributes_atime},
		    { "mtime",  config_filler::attributes_mtime},
		    {"utimes", config_filler::attributes_utimes},
		    {   "all",    config_filler::attributes_all},
		};

		for (auto it = data.begin(); not data.empty() && it != data.end(); ++it) {
			if (*it != ',' && *it != '=') {
				temp += *it;
				if (std::next(it) != data.end())
					continue;
			}

			auto itr = attributes_options.find(temp);
			if (itr != attributes_options.end()) {
				temp = "";
				if (*it == ',')
					itr->second("on");
				else if (*it == '=') {
					std::string arg;
					bool	    found_seperator = false;
					std::for_each(++it, data.end(), [&](const char c) {
						if (c != '=' && c != ',' && not found_seperator) {
							if (std::next(it) != data.end())
								++it;
							arg += c;
						} else
							found_seperator = true;
					});
					if (itr->second(arg) != 0) {
						llog::error(
						    "invalid argument '" + arg + "' for option --attributes " + itr->first + "=" + arg);
						llog::error("valid arguments: --attributes " + itr->first + "=on/off");
						exit(1);
					}
				}
			} else {
				llog::error("invalid argument '" + temp + "' for option --attributes");
				llog::error("valid arguments: --attributes atime,mtime,utimes,uid,gid,own,all");
				exit(1);
			}
		}
		return 0;
	}

	int fsync(const std::string& data) {
		if (data == "on")
			options::fsync = true;
		else if (data == "off")
			options::fsync = false;
		else
			return -1;
		return 0;
	}

	int remove_extra(const std::string& data) {
		if (data == "on")
			options::remove_extra = true;
		else if (data == "off")
			options::remove_extra = false;
		else
			return -1;
		return 0;
	}

	int update(const std::string& data) {
		if (data == "on") {
			options::update	  = true;
			options::rollback = false;
		} else if (data == "off")
			options::update = false;
		else
			return -1;
		return 0;
	}

	int rollback(const std::string& data) {
		if (data == "on") {
			options::rollback = true;
			options::update	  = false;
		} else if (data == "off")
			options::rollback = false;
		else
			return -1;
		return 0;
	}
}

namespace lunas_info {
	void author(void) {
		llog::print(about::author);
		exit(0);
	}

	void version(void) {
		llog::print(about::version);
		exit(0);
	}

	void license(void) {
		llog::print(about::license);
		exit(0);
	}

	void help(void) {
		llog::print(about::help);
		exit(0);
	}

	void meow(void) {
		llog::print("");
		llog::print("  ~-~ MeooOOoooooOOOooowwwwwW ~-~");
		llog::print("");
	}
}
