#ifndef ABOUT
#define ABOUT

#include <string>
#include "config.h"

namespace about{
	inline std::string author = 
		"\n  -> nodeluna - nodeluna@proton.me"
		"\n  -> https://github.com/nodeluna\n";

	inline std::string version = "\n  -> lunas --- ";

	inline std::string license = 
		"\n  -> GPLv3+\n"
		"  -> This program is distributed in the hope that it will be useful,\n"
		"	but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
		"	See the GNU General Public License version 3 for more details.\n"
		"  -> https://www.gnu.org/licenses/gpl-3.0.en.html";

	inline std::string help ="  -> A syncing cli tool that can handle more than two directories locally and remotely\n"
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
		"    [-s, -src, --source] </path/to/dir>\n"
		"        path of the source local directory. this option can be used many times to sync multiple directories\n"
		"\n"
		"    [-d, -dest, --destination] </path/to/dir>\n"
		"        path of the destination local directory. this option can be used many times to sync multiple directories\n"
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
		"        example: -rdest user@ip:/path/to/dir pw=PASSWORD N=NUMBER\n"
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
#else
		"    [X REMOTE X]\n"
		"        remote is disabled in the compilation process\n"
#endif // REMOTE_ENABLED
		"\n"
		"    [-mkdir, --make-directory]\n"
		"        make any input directory if it doesn't exists, locally and remotely\n"
		"\n"
		"    [-x, --exclude] <relative/path/inside/input/dir>\n"
		"        exclude a path from getting synced. this option can be used many times to *exclude* multiple directories/files\n"
		"        example: -r user@ip:/dir1/dir2/ -x dir3/dir    # only relative path starting from inside the input dir is allowed\n"
		"        example: -p /dir1/dir2/ -x file_inside_dir2    # a /full/path or regular ./relative/paths aren't allowed, and would get ignored\n"
		"\n"
		"    [-F, --fsync]\n"
		"        synchronize the copied file with the filesystem\n"
		"\n"
		"    [-L, --dereference]\n"
		"        follow symlinks. sync the target the symlink is pointing to instead of the symlink itself\n"
		"\n"
		"    [-R, --resume]\n"
		"        resume interrupted file transfer. *READ* about it in the man page. enabled by default\n"
		"        files are copied to 'file.HASH.ls.part' before being renamed to their original name 'file'\n"
		"        to ensure finding the correct source of an interrupted file or remove an orphaned interrupted file\n"
		"\n"
		"    [-c, --config] <preset>\n"
		"        sync a preconfigured preset from the config file. Read the man page the CONFIG FILE section\n"
		"        To create a demo config run 'lunas -c DEMO_CONFIG'\n"
		"\n"
		"\x1b[1;31m    [-u, --update]   \x1b[1;0m\n"
		"        check mtime of files and re-sync the file if mtime wasn't the same. this option enables -a mtime\n"
		"        the destination file, if exists, gets removed then re-synced again. it replaces old mtime files with newer mtime ones\n"
		"        [-rb, --rollback] can be used to replace newer files with older ones\n"
		"\n"
		"    [-P, --progress]\n"
		"        enable progress bar for copied files\n"
		"\n"
		"    [-v, --verbose]\n"
		"        print the source of which files/dirs were copied from not just the destination\n"
		"\n"
		"    [-q, --quiet]\n"
		"        disable print statements and only print errors\n"
		"\n"
		"    [-dr, --dry-run]\n"
		"        outputs what will be synced without actually syncing them\n"
		"\n"
		"    [--author]\n"
		"        print the program's author\n"
		"\n"
		"    [--license]\n"
		"        print the program's license\n"
		"\n"
		"    [-h, --help]\n"
		"        print this help statement\n"
		"\n"
		"\x1b[1;31m  usage\n"
		"	NOT READY!\n\x1b[1;0m";

	inline std::string smol_help ="  -> A syncing cli tool that can handle more than two directories locally and remotely\n"
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
		"    [-s, -src, --source] </path/to/dir>\n"
		"        path of the source local directory. this option can be used many times to sync multiple directories\n"
		"\n"
		"    [-d, -dest, --destination] </path/to/dir>\n"
		"        path of the destination local directory. this option can be used many times to sync multiple directories\n"
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
		"        example: -rdest user@ip:/path/to/dir pw=PASSWORD N=NUMBER\n"
		"\n"
		"    [-rs, -rsrc, --remote-source] <user@ip:/path/to/dir>\n"
		"        path of the remote source sftp directory. this option can be used many times to sync multiple directories\n"
		"\n"
		"    [-rd, -rdest, --remote-destination] <user@ip:/path/to/dir>\n"
		"        path of the remote destination sftp directory. this option can be used many times to sync multiple directories\n"
#else
		"    [X REMOTE X]\n"
		"        remote is disabled in the compilation process\n"
#endif // REMOTE_ENABLED
		"\n"
		"    [-h, --help]\n"
		"\x1b[1;31m        to print the full help statement for more options and information\x1b[0m\n"
		"\n"
		"\x1b[1;31m  usage\n"
		"	NOT READY!\x1b[1;0m";
}

#endif // ABOUT
