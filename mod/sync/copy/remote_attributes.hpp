#pragma once

#include <string>
#include <expected>
#include <variant>

#include "../types.hpp"
#include "error/error.hpp"
#include "sftp/sftp.hpp"
#include "attributes/attributes.hpp"

namespace lunas
{
	namespace ownership
	{
		std::expected<std::monostate, lunas::error> local_to_remote(const std::string& src, const std::string& dest,
									    const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc);

		std::expected<std::monostate, lunas::error> remote_to_local(const std::string& src, const std::string& dest,
									    const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc);

		std::expected<std::monostate, lunas::error> remote_to_remote(const std::string& src, const std::string& dest,
									     const std::unique_ptr<lunas::sftp>& src_sftp,
									     const std::unique_ptr<lunas::sftp>& dest_sftp,
									     const syncmisc&			 misc);
	}

	namespace permissions
	{
		std::expected<std::monostate, lunas::error> remote_to_local(const std::string& src, const std::string& dest,
									    const std::unique_ptr<lunas::sftp>& sftp, const syncmisc& misc);
	}

	namespace utimes
	{
		std::expected<std::monostate, lunas::error> remote(const std::string& src, const std::string& dest,
								   const std::unique_ptr<lunas::sftp>& src_sftp,
								   const std::unique_ptr<lunas::sftp>& dest_sftp, const syncmisc& misc);
	}
}
