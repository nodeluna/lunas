#ifndef CLIARGS
#define CLIARGS

#include <string>
#include <vector>
#include "base.h"

struct local_path{
	std::string path;
	short srcdest = SRCDEST;
};

inline std::vector<local_path> local_paths;

void next_arg_exists(const int& argc, const char* argv[], int i);

int fillopts(const int& argc, const char* argv[], int& index);

int cliargs(const int& argc, const char* argv[]);

#endif // CLIARGS
