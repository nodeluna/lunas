#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std;
#else
#	include <string>
#	include <string_view>
#	include <functional>
#	include <format>
#	include <any>
#endif

#include "error.hpp"

namespace lunas
{
	bool server_maybe_disconnected(const enum error_type type)
	{
		switch (type)
		{
			case error_type::sftp_genaric_failure:
			case error_type::sftp_none:
				return true;
			default:
				return false;
		}
	}

	/* add error values that could happen during a regular file copy for --remove-partials to work properly */
	bool sync_interrupted_error(const enum error_type type)
	{
		if (server_maybe_disconnected(type))
		{
			return true;
		}
		switch (type)
		{
			case error_type::attributes_space_info:
			case error_type::attributes_get:
			case error_type::attributes_set_utimes:
			case error_type::attributes_get_utimes:
			case error_type::attributes_set_permissions:
			case error_type::attributes_get_permissions:
			case error_type::attributes_set_ownership:
			case error_type::attributes_get_ownership:
			case error_type::attributes_permissions_check:
			case error_type::attributes_file_type:
			case error_type::sync_error_reading:
			case error_type::sync_error_writing:
			case error_type::sync_size_mismatch:
			case error_type::sync_get_file_size:
				return true;
			default:
				return false;
		}
	}
}

namespace lunas
{
	error::error(const std::string_view& message, const enum error_type type) : msg(message), type(type)
	{
	}

	error::error(const std::string& message) : msg(message)
	{
	}

	error::error(const enum error_type type) : type(type)
	{
	}

	error::error(std::function<void(std::string&, enum error_type&)> custom_constructor)
	{
		custom_constructor(this->msg, this->type);
	}

	error::error()
	{
	}

	void error::set_user_data(const std::any& data) noexcept
	{
		this->data = data;
	}

	[[nodiscard]] std::any& error::get_user_data() noexcept
	{
		return data;
	}

	const std::string error::message() const noexcept
	{
		return msg;
	}

	const char* error::what() const
	{
		return msg.c_str();
	}

	enum error_type error::value() const noexcept
	{
		return type;
	}
}
