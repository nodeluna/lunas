#ifndef RAII_SFTP
#define RAII_SFTP

#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>

#include <string>


#define REMOTE_BUFFER_SIZE 65536 * 2

namespace raii{
	namespace sftp{
		class session{
			sftp_session* _sftp;
			public:
				explicit session(sftp_session* sftp_);
				~session();
		};

		class channel{
			ssh_channel* _channel;
			public:
				explicit channel(ssh_channel* channel_);
				~channel();
		};

		class attributes{
			sftp_attributes* _attributes;
			public:
				explicit attributes(sftp_attributes* attr);
				~attributes();
		};

		class dir{
			sftp_dir* _dir;
			std::string _path;
			public:
				explicit dir(sftp_dir* dir, const std::string& path);
				~dir();
		};

		class file{
			sftp_file* _file;
			std::string _path;
			public:
				explicit file(sftp_file* file, const std::string& path);
				~file();
		};

		class link_target{
			char** target;
			public:
				explicit link_target(char** target);
				~link_target();
		};
		class statvfs_t{
			sftp_statvfs_t* _partition_stats;
			public:
				explicit statvfs_t(sftp_statvfs_t* parition_stats);
				~statvfs_t();
		};
	}

}

namespace sftp{
	int unlink(const sftp_session& sftp, const std::string& path);
	int rmdir(const sftp_session& sftp, const std::string& path);
	int mkdir(const sftp_session& sftp, const std::string& path, const unsigned int& perms);
	int symlink(const sftp_session& sftp, const std::string& target, const std::string& path);
	sftp_attributes attributes(const sftp_session& sftp, const std::string& path);
	std::string cmd(const ssh_session& ssh, const std::string& command, const std::string& ip);
	std::string homedir(const ssh_session& ssh, const std::string& ip);
	std::string cwd(const ssh_session& ssh, const std::string& ip);
}

#endif // REMOTE_ENALBED

#endif // RAII_SFTP
