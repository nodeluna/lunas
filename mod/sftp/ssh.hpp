#pragma once

#include <libssh/sftp.h>
#include <libssh/libssh.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <filesystem>
#	include <chrono>
#	include <thread>
#	include <print>
#	include <iostream>
#endif

#include "raii.hpp"
#include "log.hpp"
#include "stdout/stdout.hpp"

typedef int auth_response;

namespace lunas
{
	enum class ssh_log_level {
		no_log	  = SSH_LOG_NOLOG,
		warning	  = SSH_LOG_WARNING,
		protocol  = SSH_LOG_PROTOCOL,
		packet	  = SSH_LOG_PACKET,
		functions = SSH_LOG_FUNCTIONS,
	};

	struct session_data {
			~session_data()
			{
				if (not pw.empty())
				{
					pw.clear();
				}
			}

			std::string ip;
			int	    port = 22;
			std::string pw;
			std::string key_path;

			struct options {
					int	      compression_level = 0;
					int	      timeout		= 5;
					ssh_log_level log_level		= ssh_log_level::no_log;
					bool	      dry_run		= false;
			} options;
	};

	class ssh {
		protected:
			struct session_data session_data;
			ssh_session	    m_ssh = NULL;

		private:
			auth_response verify_publickey(const ssh_session& ssh, const std::string& ip);
			auth_response auth_password(const ssh_session& ssh, const std::string& pw);
			auth_response auth_publickey_manual(const ssh_session& ssh);
			auth_response auth_none(const ssh_session& ssh);
			std::string   auth_method(const int& method);
			auth_response auth_list(const ssh_session& ssh, const std::string& ip, const std::string& pw);
			int	      set_options(const ssh_session& ssh, ssh_options_e option, void* argument, const std::string& warning);

		public:
			ssh(const struct session_data& data);
			const ssh_session& get_ssh_session();
			std::string	   get_ip() const;
			std::string	   get_hostname() const;

			~ssh();
	};
}
