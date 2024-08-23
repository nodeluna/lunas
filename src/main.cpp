#include "init.h"
#include "cliargs.h"


int main(const int argc, const char* argv[]){
	cliargs(argc, argv);
	init_program();
	return 0;
}
