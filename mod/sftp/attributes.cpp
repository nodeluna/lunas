module;

#include <libssh/sftp.h>

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string_view>
#	include <string>
#	include <cstdint>
#endif

export module lunas.sftp:attributes;
export import lunas.file_types;

export namespace lunas {
	class sftp_attributes {
		private:
			::sftp_attributes attr = NULL;
			std::string	  file_path;

		public:
			sftp_attributes(const sftp_session& sftp, const std::string& path, follow_symlink follow);
			sftp_attributes(const ::sftp_attributes& attribute);
			sftp_attributes(const ::sftp_attributes& attribute, const std::string& path);

			sftp_attributes() {
			}

			lunas::file_types file_type();
			std::string	  file_type_name();
			bool		  exists();
			sftp_attributes	  release();
			std::string	  path();

			std::string    name();
			std::string    longname();
			uint32_t       flags();
			uint8_t	       type();
			std::uintmax_t file_size();
			uint32_t       uid();
			uint32_t       gid();
			std::string    owner();
			std::string    group();
			uint32_t       permissions();
			uint64_t       atime64();
			uint32_t       atime();
			uint32_t       atime_nseconds();
			uint64_t       createtime();
			uint32_t       createtime_nseconds();
			uint64_t       mtime64();
			uint32_t       mtime();
			uint32_t       mtime_nseconds();
			std::string    acl();
			uint32_t       extended_count();
			std::string    extended_type();
			std::string    extended_data();

			~sftp_attributes();
	};
}

namespace lunas {
	sftp_attributes::sftp_attributes(const sftp_session& sftp, const std::string& path, follow_symlink follow) {
		file_path = path;

		if (follow == follow_symlink::yes)
			attr = sftp_stat(sftp, path.c_str());
		else
			attr = sftp_lstat(sftp, path.c_str());
	}

	sftp_attributes::sftp_attributes(const ::sftp_attributes& attribute) : attr(attribute) {
	}

	sftp_attributes::sftp_attributes(const ::sftp_attributes& attribute, const std::string& path) : attr(attribute) {
		file_path = path;
	}

	std::string sftp_attributes::path() {
		return file_path;
	}

	std::string sftp_attributes::name() {
		return attr->name ? attr->name : "";
	}

	std::string sftp_attributes::longname() {
		return attr->longname ? attr->name : "";
	}

	uint32_t sftp_attributes::flags() {
		return attr->flags;
	}

	uint8_t sftp_attributes::type() {
		return attr->type;
	}

	lunas::file_types sftp_attributes::file_type() {
		if (attr == nullptr)
			return lunas::file_types::not_found;
		if (attr->type == SSH_FILEXFER_TYPE_SYMLINK)
			return lunas::file_types::symlink;
		else if (attr->type == SSH_FILEXFER_TYPE_DIRECTORY)
			return lunas::file_types::directory;
		else if (attr->type == SSH_FILEXFER_TYPE_REGULAR)
			return lunas::file_types::regular_file;
		else
			return lunas::file_types::other;
	}

	std::string sftp_attributes::file_type_name() {
		auto type = this->file_type();

		if (type == lunas::file_types::symlink)
			return "symlink";
		else if (type == lunas::file_types::directory)
			return "directory";
		else if (type == lunas::file_types::regular_file)
			return "regular_file";
		else
			return "other";
	}

	std::uintmax_t sftp_attributes::file_size() {
		return attr->size;
	}

	uint32_t sftp_attributes::uid() {
		return attr->uid;
	}

	uint32_t sftp_attributes::gid() {
		return attr->gid;
	}

	std::string sftp_attributes::owner() {
		return attr->owner;
	}

	std::string sftp_attributes::group() {
		return attr->group;
	}

	uint32_t sftp_attributes::permissions() {
		return attr->permissions;
	}

	uint64_t sftp_attributes::atime64() {
		return attr->atime64;
	}

	uint32_t sftp_attributes::atime() {
		return attr->atime;
	}

	uint32_t sftp_attributes::atime_nseconds() {
		return attr->atime_nseconds;
	}

	uint64_t sftp_attributes::createtime() {
		return attr->createtime;
	}

	uint32_t sftp_attributes::createtime_nseconds() {
		return attr->createtime_nseconds;
	}

	uint64_t sftp_attributes::mtime64() {
		return attr->mtime64;
	}

	uint32_t sftp_attributes::mtime() {
		return attr->mtime;
	}

	uint32_t sftp_attributes::mtime_nseconds() {
		return attr->mtime_nseconds;
	}

	std::string sftp_attributes::acl() {
		return attr->acl ? ssh_string_get_char(attr->acl) : "";
	}

	uint32_t sftp_attributes::extended_count() {
		return attr->extended_count;
	}

	std::string sftp_attributes::extended_type() {
		return attr->acl ? ssh_string_get_char(attr->extended_type) : "";
	}

	std::string sftp_attributes::extended_data() {
		return attr->acl ? ssh_string_get_char(attr->extended_data) : "";
	}

	bool sftp_attributes::exists() {
		return attr != NULL;
	}

	sftp_attributes sftp_attributes::release() {
		sftp_attributes temp = attr;
		attr		     = nullptr;
		return temp;
	}

	sftp_attributes::~sftp_attributes() {
		if (attr != nullptr && attr != NULL)
			sftp_attributes_free(attr);
	}

}
