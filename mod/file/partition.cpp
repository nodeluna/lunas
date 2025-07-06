module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <filesystem>
#	include <memory>
#	include <expected>
#	include <variant>
#	include <system_error>
#endif

export module lunas.file:partition;

export import lunas.sftp;
export import lunas.error;
import lunas.path;

export namespace lunas
{
	class partition {
		private:
			std::variant<std::unique_ptr<lunas::sftp_partition>, std::filesystem::space_info> _partition;

		public:
			partition(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path);
			std::uintmax_t available();
			std::uintmax_t capacity();
	};

	std::expected<std::unique_ptr<lunas::partition>, lunas::error> get_partition(const std::unique_ptr<lunas::sftp>& sftp,
										     const std::filesystem::path&	 path);
}

namespace lunas
{
	partition::partition(const std::unique_ptr<lunas::sftp>& sftp, const std::filesystem::path path)
	{
		if (sftp != nullptr)
		{
			auto ok = sftp->sftp_partition(path);
			if (not ok && ok.error().value() == lunas::error_type::no_such_file)
			{
				ok = sftp->sftp_partition(lunas::path::parent_directory(path));
			}

			if (ok)
			{
				_partition = std::move(ok.value());
			}
			else
			{
				throw ok.error();
			}
		}
		else
		{
			std::error_code ec;
			_partition = std::filesystem::space(path, ec);
			if (ec && ec == std::errc::no_such_file_or_directory)
			{
				_partition = std::filesystem::space(lunas::path::parent_directory(path), ec);
			}

			if (ec)
			{
				auto error_type = ec == std::errc::no_such_file_or_directory ? lunas::error_type::no_such_file
											     : lunas::error_type::partition_info;
				throw lunas::error("couldn't get patition info '" + path.string() + ", " + ec.message(), error_type);
			}
		}
	}

	std::uintmax_t partition::available()
	{
		if (std::holds_alternative<std::unique_ptr<lunas::sftp_partition>>(_partition))
		{
			return std::get<std::unique_ptr<lunas::sftp_partition>>(_partition)->available();
		}
		else
		{
			return std::get<std::filesystem::space_info>(_partition).available;
		}
	}

	std::uintmax_t partition::capacity()
	{
		if (std::holds_alternative<std::unique_ptr<lunas::sftp_partition>>(_partition))
		{
			return std::get<std::unique_ptr<lunas::sftp_partition>>(_partition)->capacity();
		}
		else
		{
			return std::get<std::filesystem::space_info>(_partition).capacity;
		}
	}

	std::expected<std::unique_ptr<lunas::partition>, lunas::error> get_partition(const std::unique_ptr<lunas::sftp>& sftp,
										     const std::filesystem::path&	 path)
	{
		try
		{
			return std::make_unique<lunas::partition>(sftp, path);
		}
		catch (const lunas::error& e)
		{
			return std::unexpected(e);
		}
	}
}
