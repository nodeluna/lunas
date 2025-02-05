module;

#include <string>
#include <unordered_set>
#include <cstdint>

export module lunas.config.options;
export import lunas.file_types;
import lunas.sftp;

export namespace lunas {
	namespace config {
		struct path_options {
				bool	       quiet		 = false;
				bool	       verbose		 = false;
				bool	       mkdir		 = false;
				bool	       fsync		 = false;
				bool	       no_broken_symlink = false;
				bool	       resume		 = true;
				bool	       checksum		 = false;
				bool	       compression	 = false;
				int	       compression_level = 5;
				bool	       attributes_uid	 = false;
				bool	       attributes_gid	 = false;
				bool	       attributes_mtime	 = true;
				bool	       attributes_atime	 = false;
				std::uintmax_t minimum_space	 = 1073741824;
		};

		struct options : public path_options {
				std::unordered_set<std::string> exclude;
				std::unordered_set<std::string> exclude_pattern;
				bool				dry_run		  = false;
				bool				mkdir		  = false;
				bool				progress_bar	  = true;
				bool				remove_extra	  = false;
				lunas::follow_symlink		follow_symlink	  = lunas::follow_symlink::no;
				bool				mtime_count	  = true;
				bool				no_broken_symlink = false;
				bool				update		  = false;
				bool				resume		  = true;
				bool				rollback	  = false;
				time_t				timeout_sec	  = 5;
				lunas::ssh_log_level		ssh_log_level	  = lunas::ssh_log_level::no_log;
		};
	}
}
