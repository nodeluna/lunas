#ifndef CLIARGS
#define CLIARGS

#include <string>
#include <vector>
#include <cstdint>
#include "base.h"
#include "config.h"

struct input_path {
	std::string path;
#ifdef REMOTE_ENABLED
	std::string password;
	std::string ip;
	int port = 22;
	sftp_session sftp = nullptr;
#endif // REMOTE_ENABLED
	bool remote = false;
	short srcdest;
	std::uintmax_t available_space = 0;
	std::uintmax_t synced_size = 0;
	std::uintmax_t synced_files = 0;
	std::uintmax_t synced_dirs = 0;
	std::uintmax_t removed_files = 0;
	std::uintmax_t removed_dirs = 0;
};

inline std::vector<input_path> input_paths;

bool is_num(const std::string& x);

void next_arg_exists(const int& argc, const char* argv[], int i);

int fillopts(const int& argc, const char* argv[], int& index);

int cliargs(const int& argc, const char* argv[]);

#endif // CLIARGS
