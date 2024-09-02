#include "init.h"
#include "cliargs.h"
#include "about.h"
#include "log.h"


int main(const int argc, const char* argv[]){
	if(argc == 1)
		llog::print(about::smol_help);
	cliargs(argc, argv);
	init_program();
	return 0;
}
