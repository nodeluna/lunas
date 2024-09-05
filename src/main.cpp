#include "init.h"
#include "cliargs.h"
#include "about.h"
#include "log.h"
#include "config_manager.h"


int main(const int argc, const char* argv[]){
	if(argc == 1){
		llog::print(about::smol_help);
		exit(1);
	}

	config_manager::preset("global");
	cliargs(argc, argv);
	init_program();
	return 0;
}
