module;

#include <string_view>
#include <string>
#include <cstdint>
#include <libssh/sftp.h>

export module sftp:attributes;

export namespace lunas {
	enum class follow_symlink {
		yes,
		no,
	};

	enum class file_type {
		regular_file,
		directory,
		symlink,
		socket,
		block_file,
		character_file,
		fifo,
		other,
	};

	class attributes {
		private:
			::sftp_attributes attr = nullptr;
			std::string	  file_path;

		public:
			attributes(const sftp_session& sftp, const std::string& path, follow_symlink follow);
			file_type	file_type();
			std::string	file_type_name();
			bool		exists();
			sftp_attributes release();
			std::string	path();

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

			~attributes();
	};
}

namespace lunas {
	attributes::attributes(const sftp_session& sftp, const std::string& path, follow_symlink follow) {
		file_path = path;

		if (follow == follow_symlink::yes)
			attr = sftp_stat(sftp, path.c_str());
		else
			attr = sftp_lstat(sftp, path.c_str());
	}

	std::string attributes::path() {
		return file_path;
	}

	std::string attributes::name() {
		return attr->name ? attr->name : "";
	}

	std::string attributes::longname() {
		return attr->longname ? attr->name : "";
	}

	uint32_t attributes::flags() {
		return attr->flags;
	}

	uint8_t attributes::type() {
		return attr->type;
	}

	file_type attributes::file_type() {
		if (attr->type == SSH_FILEXFER_TYPE_SYMLINK)
			return file_type::symlink;
		else if (attr->type == SSH_FILEXFER_TYPE_DIRECTORY)
			return file_type::directory;
		else if (attr->type == SSH_FILEXFER_TYPE_REGULAR)
			return file_type::regular_file;
		else
			return file_type::other;
	}

	std::string attributes::file_type_name() {
		auto type = this->file_type();

		if (type == file_type::symlink)
			return "symlink";
		else if (type == file_type::directory)
			return "directory";
		else if (type == file_type::regular_file)
			return "regular_file";
		else
			return "other";
	}

	std::uintmax_t attributes::file_size() {
		return attr->size;
	}

	uint32_t attributes::uid() {
		return attr->uid;
	}

	uint32_t attributes::gid() {
		return attr->gid;
	}

	std::string attributes::owner() {
		return attr->owner;
	}

	std::string attributes::group() {
		return attr->group;
	}

	uint32_t attributes::permissions() {
		return attr->permissions;
	}

	uint64_t attributes::atime64() {
		return attr->atime64;
	}

	uint32_t attributes::atime() {
		return attr->atime;
	}

	uint32_t attributes::atime_nseconds() {
		return attr->atime_nseconds;
	}

	uint64_t attributes::createtime() {
		return attr->createtime;
	}

	uint32_t attributes::createtime_nseconds() {
		return attr->createtime_nseconds;
	}

	uint64_t attributes::mtime64() {
		return attr->mtime64;
	}

	uint32_t attributes::mtime() {
		return attr->mtime;
	}

	uint32_t attributes::mtime_nseconds() {
		return attr->mtime_nseconds;
	}

	std::string attributes::acl() {
		return attr->acl ? ssh_string_get_char(attr->acl) : "";
	}

	uint32_t attributes::extended_count() {
		return attr->extended_count;
	}

	std::string attributes::extended_type() {
		return attr->acl ? ssh_string_get_char(attr->extended_type) : "";
	}

	std::string attributes::extended_data() {
		return attr->acl ? ssh_string_get_char(attr->extended_data) : "";
	}

	bool attributes::exists() {
		return attr != nullptr;
	}

	sftp_attributes attributes::release() {
		sftp_attributes temp = attr;
		attr		     = nullptr;
		return temp;
	}

	attributes::~attributes() {
		if (attr != nullptr && attr != NULL)
			sftp_attributes_free(attr);
	}

}
