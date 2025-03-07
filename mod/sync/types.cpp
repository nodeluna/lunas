module;

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
#endif

export module lunas.sync:types;
export import lunas.file_types;
export import lunas.sftp;
export import lunas.config.options;
export import lunas.stats;
export import lunas.terminal;
export import lunas.error;

export namespace lunas {
	enum class sync_code {
		none,
		success,
		post_sync_fail,
		failed,
		interrupted,
	};

	class progress_bar {
		private:
			bool lock     = false;
			bool quiet    = false;
			bool progress = false;

		public:
			progress_bar(bool progress, bool quiet);
			void bar(const double& full_size, const double& occupied);
			void ingoing(const double& full_size, const double& occupied);
			~progress_bar();
	};

	struct buffque {
			std::vector<char>					      buffer;
			int							      bytes_xfered = 0;
			std::expected<std::unique_ptr<lunas::sftp_aio>, lunas::error> aio;

			explicit buffque(std::uint64_t size) : buffer(size) {
			}
	};

	struct syncstat {
			enum sync_code code	   = lunas::sync_code::none;
			std::uintmax_t copied_size = 0;
	};

	struct progress_stats {
			std::uintmax_t total_synced	  = 1;
			std::uintmax_t total_to_be_synced = 0;
	};

	struct syncmisc {
			const time_t		      src_mtime = 0;
			const lunas::file_types	      file_type = lunas::file_types::not_found;
			const lunas::config::options& options;
			const lunas::progress_stats   progress_stats;
	};
}

namespace lunas {
	progress_bar::progress_bar(bool progress, bool quiet) {
		this->quiet    = quiet;
		this->progress = progress;
		if (progress == false || quiet)
			return;
		std::cout << '\n';
	}

	void progress_bar::bar(const double& full_size, const double& occupied) {
		std::string	stat		= "\r(" + lunas::size_units(occupied) + "/" + lunas::size_units(full_size) + ") [";
		struct termsize term		= lunas::terminal_size();
		float		term_width	= term.ts_col;
		std::string	size_percentage = " " + lunas::decimal_precision((occupied / full_size) * 100, 2) + " %";
		double		bar_width	= term_width - stat.size() - size_percentage.size();
		int		occupied_width	= bar_width * (occupied / full_size);
		int		free_width	= bar_width * (1 - (occupied / full_size));

		std::cout << "\x1b[1;32m" << std::left << std::setw(stat.size()) << stat << "\x1b[1;0m";
		std::cout << std::setfill('#') << std::setw(occupied_width) << "" << std::flush;
		std::cout << std::setfill('~') << std::setw(free_width) << "" << std::flush;
		std::cout << "] ";
		std::cout << "\x1b[1;32m" << size_percentage << "\x1b[1;0m";
		lock = true;
	}

	void progress_bar::ingoing(const double& full_size, const double& occupied) {
		if (progress == false || quiet)
			return;
		std::cout << "\x1b[1B\r";
		this->bar(full_size, occupied);
		std::cout << "\x1b[1A\r";
	}

	progress_bar::~progress_bar() {
		if (progress == false || quiet)
			return;

		if (lock) {
			std::cout << "\x1b[2K";
			std::cout << "\x1b[1B\r";
			std::cout << "\x1b[2K";
			std::cout << "\x1b[2A";
			std::cout << "\n\n\n\n\n";
			std::cout << "\x1b[6A";
		} else
			std::cout << "\x1b[1A";

		lock = false;
	}
}
