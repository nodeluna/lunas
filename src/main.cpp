#include "init.h"
#include "cliargs.h"
#include "about.h"
#include "log.h"
#include "config_manager.h"

int main(const int argc, const char* argv[]) {
	config_manager::preset("global");

	cliargs(argc, argv);
	if (input_paths.size() < 2) {
		llog::print(about::smol_help);
		exit(1);
	}
	init_program();
	return 0;
}
