#include <unistd.h>
#include <sys/ioctl.h>
#include "term.h"


struct winsize terminal_size(void){
	struct winsize term_size;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &term_size);
	return term_size;
}
