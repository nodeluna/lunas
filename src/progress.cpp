#include <string>
#include <iostream>
#include <iomanip>
#include "term.h"
#include "size_units.h"
#include "progress.h"
#include "config.h"


namespace progress {
	void bar(const double& full_size, const double& occupied){
		std::string stat = "\r(" + size_units(occupied) + "/" + size_units(full_size) + ") [";

		struct winsize term = terminal_size();
		float term_width = term.ws_col;

		std::string size_percentage = " " + decimal_precision((occupied / full_size) * 100, 2) + " %";

		double bar_width = term_width - stat.size() - size_percentage.size();
		int occupied_width = bar_width * (occupied / full_size);
		int free_width = bar_width * (1 - (occupied / full_size));

		std::cout << "\x1b[1;32m" << std::left << std::setw(stat.size()) << stat << "\x1b[1;0m";
		std::cout << std::setfill('#') << std::setw(occupied_width) << "" << std::flush;
		std::cout << std::setfill('~') << std::setw(free_width) << "" << std::flush;
		std::cout << "] ";
		std::cout << "\x1b[1;32m" << size_percentage << "\x1b[1;0m";
		std::cout << "\n";
	}

	void prepare(void){
		if(options::progress_bar == false)
			return;
		std::cout << '\n';
	}

	void ingoing(const double& full_size, const double& occupied){
		if(options::progress_bar == false)
			return;
		std::cout << "\x1b[1B\r";
		progress::bar(full_size, occupied);
		std::cout << "\x1b[2A\r";

	}

	void reset(void){
		if(options::progress_bar == false)
			return;
		std::cout << "\x1b[2K";
		std::cout << "\x1b[1B\r";
		std::cout << "\x1b[2K";
		std::cout << "\x1b[2A";
		std::cout << "\n\n\n\n\n";
		std::cout << "\x1b[6A";
	}
}
