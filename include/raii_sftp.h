#ifndef RAII_SFTP
#define RAII_SFTP

#include "config.h"

#ifdef REMOTE_ENABLED
#include <libssh/sftp.h>

#include <string>
#include <expected>

typedef int SSH_STATUS;

#define REMOTE_BUFFER_SIZE 65536 * 2

namespace raii{
	namespace sftp{
		class session{
			sftp_session* _sftp;
			public:
				explicit session(sftp_session* sftp_);
				session(const session&) = delete;
				session& operator=(const session&) = delete;
				~session();
		};

		class channel{
			ssh_channel* _channel;
			public:
				explicit channel(ssh_channel* channel_);
				channel(const channel&) = delete;
				channel& operator=(const channel&) = delete;
				~channel();
		};

		class attributes{
			sftp_attributes* _attributes;
			public:
				explicit attributes(sftp_attributes* attr);
				attributes(const attributes&) = delete;
				attributes& operator=(const attributes&) = delete;
				~attributes();
		};

		class dir{
			sftp_dir* _dir;
			std::string _path;
			public:
				explicit dir(sftp_dir* dir, const std::string& path);
				dir(const dir&) = delete;
				dir& operator=(const dir&) = delete;
				~dir();
		};

		class file{
			sftp_file* _file;
			std::string _path;
			public:
				explicit file(sftp_file* file, const std::string& path);
				file(const file&) = delete;
				file& operator=(const file&) = delete;
				~file();
		};

		class link_target{
			char** target;
			public:
				explicit link_target(char** target);
				link_target(const link_target&) = delete;
				link_target& operator=(const link_target&) = delete;
				~link_target();
		};
		class statvfs_t{
			sftp_statvfs_t* _partition_stats;
			public:
				explicit statvfs_t(sftp_statvfs_t* parition_stats);
				statvfs_t(const statvfs_t&) = delete;
				statvfs_t& operator=(const statvfs_t&) = delete;
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
	std::expected<std::string, SSH_STATUS> cmd(const ssh_session& ssh, const std::string& command, const std::string& ip);
	std::expected<std::string, SSH_STATUS> readlink(const ssh_session& ssh, const std::string& link, const std::string& ip);
	std::expected<bool, SSH_STATUS> is_broken_link(const sftp_session& sftp, const std::string& link, const std::string& ip);
	std::expected<std::string, SSH_STATUS> homedir(const ssh_session& ssh, const std::string& ip);
	std::expected<std::string, SSH_STATUS> cwd(const ssh_session& ssh, const std::string& ip);
}

#endif // REMOTE_ENALBED

#endif // RAII_SFTP
