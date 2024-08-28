#ifndef ABOUT
#define ABOUT

#include <string>
#include "config.h"

namespace about{
	inline std::string author = 
		"\n  -> nodeluna - nodeluna@proton.me"
		"\n  -> https://github.com/nodeluna\n";

	inline std::string version = "\n  -> lunas --- ";

	inline std::string help =">>> A syncing cli tool that can handle more than two directories locally and remotely\n"
		"        _\n"
		"       | |  _  _   _ _    __ _   ___ \n"
		"       | | | || | | ' \\  / _` | (_-< \n"
		"       |_|  \\_,_| |_||_| \\__,_| /__/\n"
		"\n"
		"\x1b[1;31m  usage\n"
		"	NOT READY!\n\x1b[1;0m"
		"\n"
		"  options\n"
		"    [-p, --path] <dir>\n"
		"        path of the local directory to be synced. this option can be used many times to sync multiple directories\n"
		"\n"
#ifdef REMOTE_ENABLED
		"    [-r, --remote-path] <user@ip:/path/to/dir>\n"
		"        path of the remote sftp directory to be synce. this option can be used many times to sync multiple directories\n"
		"\n"
		"    [N=NUMBER, port=NUMBER]\n"
		"        the port number. if this option was used, it must come after the relevant remote path. It defaults to 22, if the option is not used\n"
		"        example: -rdest user@ip:/path/to/dir port=NUMBER\n"
		"\n"
		"    [pw=PASSWORD, password=PASSWORD]\n"
		"        password of the remote directory. if this option was used, it must come after the relevant remote path.\n"
		"        publickey authetication is used by default, if it wasn't successful you should be prompted for the password\n"
		"        example: -rdest user@ip:/path/to/dir pw=PASSWORD N=NUMBER\n"
		"\n"
#else
		"    [X REMOTE X]\n"
		"        remote is disabled in the compilation process\n"
		"\n"
#endif // REMOTE_ENABLED
		"    [-mkdir, --make-directory]\n"
		"        make any input directory if it doesn't exists, locally and remotely\n"
		"\n"
		"    [-s, -src, --source] </path/to/dir>\n"
		"        path of the source local directory. this option can be used many times to sync multiple directories\n"
		"\n"
		"    [-d, -dest, --destination] </path/to/dir>\n"
		"        path of the destination local directory. this option can be used many times to sync multiple directories\n"
#ifdef REMOTE_ENABLED
		"\n"
		"    [-rs, -rsrc, --remote-source] <user@ip:/path/to/dir>\n"
		"        path of the remote source sftp directory. this option can be used many times to sync multiple directories\n"
		"\n"
		"    [-rd, -rdest, --remote-destination] <user@ip:/path/to/dir>\n"
		"        path of the remote destination sftp directory. this option can be used many times to sync multiple directories\n"
		"\n"
		"    [-C, --compression]\n"
		"        enable compression in remote transfer, zlib is used\n"
		"\n"
		"    [-CL, --compression-level]\n"
		"        chose the compression level, 1-9, 9 being the most efficient but slower\n"
#endif // REMOTE_ENABLED
		"\n"
		"\x1b[1;31m    [-u, --update]   \x1b[1;0m\n"
		"        check mtime of files and re-sync the file if mtime wasn't the same. this option enables -a mtime\n"
		"        the destination file, if exists, gets removed then re-synced again. it replaces old mtime files with newer mtime ones\n"
		"        [-rb, --rollback] can be used to replace newer files with older ones\n"
		"\n"
		"    [-dr, --dry-run]\n"
		"        outputs what will be synced without actually syncing them\n"
		"\n"
		"    [--author]\n"
		"        print the program's author\n"
		"\n"
		"    [-h, --help]\n"
		"        print this help statement\n"
		"\n"
		"\x1b[1;31m  usage\n"
		"	NOT READY!\n\x1b[1;0m";
}

#endif // ABOUT
