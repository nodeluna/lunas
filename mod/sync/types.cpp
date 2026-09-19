#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <cstdint>
#	include <ctime>
#	include <vector>
#	include <memory>
#	include <expected>
#	include <string>
#	include <iostream>
#	include <iomanip>
#	include <iostream>
#	include <filesystem>
#endif

#include "types.hpp"

namespace lunas
{
	progress_bar::progress_bar(bool progress, bool quiet)
	{
		this->quiet    = quiet;
		this->progress = progress;
		if (progress == false || quiet)
		{
			return;
		}
		std::cout << '\n';
	}

	void progress_bar::bar(const double& full_size, const double& occupied)
	{
		std::string	stat		= "\r(" + lunas::size_units(occupied) + "/" + lunas::size_units(full_size) + ") [";
		struct termsize term		= lunas::terminal_size();
		unsigned short	term_width	= term.ts_col;
		std::string	size_percentage = " " + lunas::decimal_precision((occupied / full_size) * 100, 2) + " %";
		unsigned long	bar_width	= term_width - (stat.size() + size_percentage.size());
		int		occupied_width	= bar_width * (occupied / full_size);
		int		free_width	= bar_width * (1 - (occupied / full_size));

		std::cout << "\x1b[1;32m" << std::left << std::setw(stat.size()) << stat << "\x1b[1;0m";
		std::cout << std::setfill('#') << std::setw(occupied_width) << "" << std::flush;
		std::cout << std::setfill('~') << std::setw(free_width) << "" << std::flush;
		std::cout << "] ";
		std::cout << "\x1b[1;32m" << size_percentage << "\x1b[1;0m";
		lock = true;
	}

	void progress_bar::ingoing(const double& full_size, const double& occupied)
	{
		if (progress == false || quiet)
		{
			return;
		}
		std::cout << "\x1b[1B\r";
		this->bar(full_size, occupied);
		std::cout << "\x1b[1A\r";
	}

	progress_bar::~progress_bar()
	{
		if (progress == false || quiet)
		{
			return;
		}

		if (lock)
		{
			std::cout << "\x1b[2K";
			std::cout << "\x1b[1B\r";
			std::cout << "\x1b[2K";
			std::cout << "\x1b[2A";
			std::cout << "\n\n\n\n\n";
			std::cout << "\x1b[6A";
		}
		else
		{
			std::cout << "\x1b[1A";
		}

		lock = false;
	}

	bool no_ownership_value(const lunas::config::options& options)
	{
		if (not options.attributes_uid_value && options.attributes_uid)
		{
			return true;
		}
		else if (not options.attributes_gid_value && options.attributes_gid)
		{
			return true;
		}
		return false;
	}

	int ownership_value(const lunas::config::options& options, int uid, enum ownership_type type)
	{
		if (type == ownership_type::uid)
		{
			if (not options.attributes_uid)
			{
				return -1;
			}
			else if (options.attributes_uid_value)
			{
				return *options.attributes_uid_value;
			}
			else
			{
				return uid;
			}
		}
		else if (type == ownership_type::gid)
		{
			if (not options.attributes_gid)
			{
				return -1;
			}
			else if (options.attributes_gid_value)
			{
				return *options.attributes_gid_value;
			}
			else
			{
				return uid;
			}
		}
		else
		{
			return -1;
		}
	}
}
