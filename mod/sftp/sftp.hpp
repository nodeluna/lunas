#pragma once

#include <libssh/sftp.h>
#include <libssh/libssh.h>

#include <string_view>
#include <string>
#include <memory>
#include <expected>
#include <filesystem>
#include <variant>

#include "attributes.hpp"
#include "ssh.hpp"
#include "log.hpp"
#include "error.hpp"
#include "dir.hpp"
#include "file.hpp"
#include "limits.hpp"
#include "partition.hpp"

#define REMOTE_BUFFER_SIZE 65536 * 2

namespace lunas
{
	class sftp : public ssh {
		protected:
			::sftp_session m_sftp = NULL;

		public:
			enum class time_type : uint8_t {
				atime  = 1,
				mtime  = 2,
				utimes = 3,
			};

			struct time_val {
					time_t atime	  = 0;
					time_t atime_nsec = 0;
					time_t mtime	  = 0;
					time_t mtime_nsec = 0;
			};

			struct owner {
					int uid = -1;
					int gid = -1;
			};

			sftp(const struct session_data& data);
			~sftp();

			const sftp_session&			    get_sftp_session();

			std::expected<std::monostate, lunas::error> unlink(const std::string& path);
			std::expected<std::monostate, lunas::error> rmdir(const std::string& path);
			std::expected<std::monostate, lunas::error> mkdir(const std::string& path, const unsigned int& perms = 0755);
			std::expected<std::monostate, lunas::error> mkdir(const std::string& path, const std::filesystem::perms& perms);
			std::expected<std::monostate, lunas::error> symlink(const std::string& target, const std::string& path);
			std::expected<std::monostate, lunas::error> hardlink(const std::string& target, const std::string& path);
			std::expected<std::monostate, lunas::error> rename(const std::string& original, const std::string& newname);
			std::expected<std::unique_ptr<lunas::sftp_partition>, lunas::error> sftp_partition(const std::string& path);
			std::expected<std::unique_ptr<lunas::sftp_dir>, lunas::error>	    opendir(const std::string& path);
			std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error>	 openfile(const std::string& path, int access_type,
												  mode_t mode);
			std::expected<std::unique_ptr<lunas::sftp_file>, lunas::error>	 openfile(const std::string& path, int access_type,
												  std::filesystem::perms mode);
			std::expected<std::unique_ptr<lunas::sftp_limits>, lunas::error> limits();
			std::expected<std::string, lunas::error>			 cmd(const std::string& command);
			std::expected<std::string, lunas::error>			 readlink(const std::string& link);
			std::expected<bool, lunas::error>				 is_broken_link(const std::string& link);
			std::expected<std::string, lunas::error>			 homedir();
			std::expected<std::string, lunas::error>			 homedir(const std::string_view& user);
			std::expected<std::string, lunas::error>			 cwd();
			std::expected<std::string, lunas::error>			 absolute_path();
			std::expected<std::uintmax_t, lunas::error>			 file_size(const std::string& path);

			template<typename struct_time_val = time_val, typename struct_time_type = time_type>
			std::expected<struct_time_val, lunas::error> get_utimes(const std::string& path, const struct_time_type time_type,
										const lunas::follow_symlink follow)
			{

				auto attr = this->attributes(path, follow);
				if (not attr)
				{
					return std::unexpected(attr.error());
				}
				auto&		attributes = attr.value();

				struct_time_val time_val;
				struct timespec timespec;
				int		rv;

				switch (time_type)
				{
					case struct_time_type::atime:
						time_val.atime	    = attributes->atime();
						time_val.atime_nsec = attributes->atime_nseconds();

						rv		    = clock_gettime(CLOCK_REALTIME, &timespec);
						if (rv == 0)
						{
							time_val.mtime	    = timespec.tv_sec;
							time_val.mtime_nsec = timespec.tv_nsec;
						}
						else
						{
							return std::unexpected(lunas::ssh_error(
							    this->get_sftp_session(), fmt::err_path("couldn't get utimes", path)));
						}
						break;
					case struct_time_type::mtime:
						time_val.mtime	    = attributes->mtime();
						time_val.mtime_nsec = attributes->mtime_nseconds();

						rv		    = clock_gettime(CLOCK_REALTIME, &timespec);
						if (rv == 0)
						{
							time_val.atime	    = timespec.tv_sec;
							time_val.atime_nsec = timespec.tv_nsec;
						}
						else
						{
							return std::unexpected(lunas::ssh_error(
							    this->get_sftp_session(), fmt::err_path("couldn't get utimes", path)));
						}
						break;
					case struct_time_type::utimes:
						time_val.atime	    = attributes->atime();
						time_val.atime_nsec = attributes->atime_nseconds();
						time_val.mtime	    = attributes->mtime();
						time_val.mtime_nsec = attributes->mtime_nseconds();
						break;
					default:
						break;
				}

				return time_val;
			}

			template<typename struct_time_val = time_val>
			std::expected<std::monostate, lunas::error> set_utimes(const std::string& path, const struct_time_val& time_val,
									       const lunas::follow_symlink follow)
			{

				struct sftp_attributes_struct attributes;
				attributes.flags	  = SSH_FILEXFER_ATTR_ACMODTIME;
				attributes.atime	  = time_val.atime;
				attributes.atime_nseconds = time_val.atime_nsec;
				attributes.mtime	  = time_val.mtime;
				attributes.mtime_nseconds = time_val.mtime_nsec;

				int rc			  = SSH_OK;

				if (follow == lunas::follow_symlink::yes)
				{
					rc = sftp_setstat(m_sftp, path.c_str(), &attributes);
				}
				else
				{
					rc = sftp_lsetstat(m_sftp, path.c_str(), &attributes);
				}

				if (rc != SSH_OK)
				{
					return std::unexpected(
					    lunas::ssh_error(this->get_sftp_session(), fmt::err_path("couldn't set utimes", path)));
				}

				return std::monostate();
			}

			std::expected<uint32_t, lunas::error> get_permissions(const std::string& path, const lunas::follow_symlink follow);

			std::expected<sftp::owner, lunas::error> get_ownership(const std::string& path, const lunas::follow_symlink follow);
			std::expected<std::monostate, lunas::error> set_ownership(const std::string& path, const sftp::owner own,
										  const lunas::follow_symlink follow);
			std::expected<std::unique_ptr<lunas::sftp_attributes>, lunas::error> attributes(const std::string& path,
													follow_symlink	   type);

			std::string							     get_str_error();
			int								     get_error_code();
			lunas::error							     get_error(const std::string& msg);
	};
}
