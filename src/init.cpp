#include <cstdlib>
#include <filesystem>
#include <limits>
#include <iostream>
#include <string>
#include <vector>
// #include <cmath>
#include <set>
#include "config.h"

#ifdef REMOTE_ENABLED
#	include <libssh/sftp.h>
#	include "remote_session.h"
#	include "fs_remote.h"
#endif // REMOTE_ENABLED

#include "init.h"
#include "cliargs.h"
#include "base.h"
#include "fs_local.h"
#include "log.h"
#include "os.h"
#include "cppfs.h"
#include "copy.h"
#include "size_units.h"
#include "resume.h"
#include "partition.h"

#ifdef REMOTE_ENABLED
void init_rsessions(void) {
	for (auto& remote_path : input_paths) {
		if (remote_path.remote == false)
			continue;
		ssh_session ssh	 = rsession::init_ssh(remote_path.ip, remote_path.port, remote_path.password);
		remote_path.sftp = rsession::init_sftp(ssh, remote_path.ip);
		remote_path.path = rsession::absolute_path(remote_path.sftp, remote_path.ip);
		if (not remote_path.password.empty())
			remote_path.password.clear();
	}
}
#endif // REMOTE_ENABLED

void fill_base(void) {
	unsigned long int index_path = 0;
	for (auto& i : input_paths) {
		llog::print("--> reading directory " + input_paths.at(index_path).path);
		if (i.remote == false) {
			fs_local::list_tree(i, index_path);
			auto space = fs_local::available_space(i);
			if (not space)
				llog::ec(i.path, space.error(), "couldn't get available space", EXIT_FAILURE);
			i.available_space = space.value();
		}
#ifdef REMOTE_ENABLED
		else {
			fs_remote::list_tree(i, index_path);
			auto space = fs_remote::available_space(i);
			if (not space)
				llog::rc(i.sftp, i.path, space.error(), "couldn't get available space", EXIT_FAILURE);
			i.available_space = space.value();
		}
#endif // REMOTE_ENABLED
		index_path++;
	}
	llog::print("");
}

unsigned long int get_src(const struct base::path& file) {
	long int	  src_mtime;
	unsigned long int src_mtime_i = std::numeric_limits<unsigned long int>::max();
	unsigned long int i	      = 0;

	bool first = true;
	for (const auto& metadata : file.metadatas) {
		if (not condition::is_src(input_paths.at(i).srcdest) || metadata.type == NON_EXISTENT)
			goto end;
		if (first) {
			first = false;
			goto assign;
		} else if (!options::rollback && src_mtime >= metadata.mtime)
			goto end;
		else if (options::rollback && src_mtime <= metadata.mtime)
			goto end;

	assign:
		src_mtime   = metadata.mtime;
		src_mtime_i = i;
	end:
		i++;
	}

	return src_mtime_i;
}

void register_sync(const struct syncstat& syncstat, const unsigned long int& dest_index, const short& type) {
	if (syncstat.code == 0)
		return;
	input_paths.at(dest_index).synced_size += syncstat.copied_size;
	input_paths.at(dest_index).available_space -= syncstat.copied_size;
	if (type == DIRECTORY)
		input_paths.at(dest_index).synced_dirs++;
	else
		input_paths.at(dest_index).synced_files++;
	base::syncing_counter++;
}

#ifdef REMOTE_ENABLED
std::expected<std::uintmax_t, int> file_size(const sftp_session& sftp, const std::string& path, const short& type) {
	if (type != REGULAR_FILE)
		return 0;
	if (sftp != nullptr) {
		auto size = sftp::file_size(sftp, path);
		if (not size) {
			llog::rc(sftp, path, size.error(), "couldn't get file size", NO_EXIT);
			return std::unexpected(-1);
		}
		return size.value();
	} else
#else
std::expected<std::uintmax_t, int> file_size(const std::string& path, const short& type) {
	if (type != REGULAR_FILE)
		return 0;
#endif // REMOTE_ENABLED
	{
		auto size = cppfs::file_size(path);
		if (not size) {
			llog::ec(path, size.error(), "couldn't get file size", NO_EXIT);
			return std::unexpected(-1);
		}
		return size.value();
	}
}

bool check_partition_available_space(const struct input_path& input_path, const std::uintmax_t& size) {
	if (partition::getting_full(input_path.available_space, size))
		return false;
	return true;
}

bool is_there_space_left(const size_t& src_mtime_i, const size_t& dest_index, const std::string& src, const short& type) {
#ifdef REMOTE_ENABLED
	auto size = file_size(input_paths.at(src_mtime_i).sftp, src, type);
	if (not size)
		return false;
	else if (not check_partition_available_space(input_paths.at(dest_index), size.value())) {
		return false;
	}
#else
	auto& _	   = src_mtime_i;
	auto  size = file_size(src, type);
	if (not size)
		return false;
	else if (not check_partition_available_space(input_paths.at(dest_index), size.value()))
		return false;
#endif // REMOTE_ENABLED

	return true;
}

int avoid_dest(const struct base::path& file, const struct metadata& metadata, const size_t& src_mtime_i, const size_t& dest_index) {
	const std::string src	    = input_paths.at(src_mtime_i).path + file.name;
	const long int&	  src_mtime = file.metadatas.at(src_mtime_i).mtime;
	if (src_mtime_i == dest_index)
		return SAME_INPUT_PATH;
	else if (condition::is_dest(input_paths.at(dest_index).srcdest) == false)
		return NOT_DEST;
	else if (metadata.type != NON_EXISTENT && metadata.type == DIRECTORY)
		return EXISTING_DIR;
	// else if(options::follow_symlink && metadata.type < 0 && std::abs(metadata.type) == file.metadatas.at(src_mtime_i).type){
	else if (metadata.type == BROKEN_SYMLINK && file.metadatas.at(src_mtime_i).type == SYMLINK) {
		return OK_DEST;
	} else if (metadata.type != NON_EXISTENT && (metadata.type != file.metadatas.at(src_mtime_i).type)) {
		const std::string dest = input_paths.at(dest_index).path + file.name;
		llog::warn("conflict in types between *" + get_type_name(metadata.type) + "* '" + dest + "' and *" +
			   get_type_name(file.metadatas.at(src_mtime_i).type) + "* '" + src + "'");
		llog::warn("not syncing them");
		return TYPE_CONFLICT;
	} else if (src_mtime == metadata.mtime)
		return SAME_MTIME;
	else if (not is_there_space_left(src_mtime_i, dest_index, src, file.metadatas.at(src_mtime_i).type))
		return NO_SPACE_LEFT;
	return OK_DEST;
}

void updating(const struct base::path& file, const unsigned long int& src_mtime_i) {
	const long int&	  src_mtime = file.metadatas.at(src_mtime_i).mtime;
	const short&	  type	    = file.metadatas.at(src_mtime_i).type;
	const std::string src	    = input_paths.at(src_mtime_i).path + file.name;

	unsigned long int dest_index = 0;
	bool		  sync	     = false;
	for (const auto& metadata : file.metadatas) {
		if (avoid_dest(file, metadata, src_mtime_i, dest_index) == NO_SPACE_LEFT) {
			llog::error(
			    "can't sync '" + input_paths.at(dest_index).path + file.name +
			    "' partition getting full. available space left: " + size_units(input_paths.at(dest_index).available_space));
			goto end;
		} else if (avoid_dest(file, metadata, src_mtime_i, dest_index) != OK_DEST)
			goto end;

		if (options::update && src_mtime > metadata.mtime)
			sync = true;
		else if (options::rollback && src_mtime < metadata.mtime)
			sync = true;
		else if (metadata.type == NON_EXISTENT)
			sync = true;

		if (sync) {
			struct syncmisc misc = {
			    .src_mtime = src_mtime,
			    .file_type = type,
			};
			std::string dest = input_paths.at(dest_index).path + file.name;
#ifdef REMOTE_ENABLED
			syncstat syncstat = lunas::copy(src, dest, input_paths.at(src_mtime_i).sftp, input_paths.at(dest_index).sftp, misc);
#else
			struct syncstat syncstat = lunas::copy(src, dest, misc);
#endif // REMOTE_ENABLED
			register_sync(syncstat, dest_index, type);
		}
	end:
		sync = false;
		dest_index++;
	}
}

int avoid_src(const struct base::path& file, const unsigned long int& src_mtime_i) {
	if (src_mtime_i == std::numeric_limits<unsigned long int>::max())
		return NO_SRC;
	else if (file.metadatas.at(src_mtime_i).type == NON_EXISTENT)
		return NO_SRC;
	else if (file.metadatas.at(src_mtime_i).type == BROKEN_SYMLINK)
		return BROKEN_SRC;
	return OK_SRC;
}

void syncing() {
	for (auto itr = content.begin(); itr != content.end();) {
		unsigned long int src_mtime_i = get_src(*itr);
		if (avoid_src(*itr, src_mtime_i) == NO_SRC)
			goto end;
		else if (avoid_src(*itr, src_mtime_i) == BROKEN_SRC)
			goto avoid;

		updating(*itr, src_mtime_i);

		if (!options::remove_extra)
			goto end;
	avoid:
		itr = content.erase(itr);
		continue;
	end:
		++itr;
	}
}

void remove_extra() {
	llog::print("");
	for (auto it = content.rbegin(); it != content.rend(); ++it) {
		unsigned long int src_mtime_i = get_src(*it);
		if (!avoid_src(*it, src_mtime_i))
			continue;
		size_t i = 0;
		for (const auto& file : it->metadatas) {
			if (file.type != NON_EXISTENT && input_paths.at(i).srcdest == DEST) {
				llog::print("! removing extra '" + input_paths.at(i).path + it->name + "', not found in any source");
#ifdef REMOTE_ENABLED
				auto fsize = file_size(input_paths.at(i).sftp, input_paths.at(i).path + it->name, file.type);
#else
				auto fsize = file_size(input_paths.at(i).path + it->name, file.type);
#endif // REMOTE_ENABLED
				if (fsize)
					input_paths.at(i).removed_size += fsize.value();
#ifdef REMOTE_ENABLED
				resume::unlink(input_paths.at(i).sftp, input_paths.at(i).path + it->name, file.type);
#else
				resume::unlink(input_paths.at(i).path + it->name);
#endif // REMOTE_ENABLED
				if (file.type == DIRECTORY)
					input_paths.at(i).removed_dirs++;
				else
					input_paths.at(i).removed_files++;
			}
			i++;
		}
	}
}

void counter() {
	for (const auto& file : content) {
		long int	  tmp		   = -1;
		unsigned long int files_dont_exist = 0;
		for (const auto& metadata : file.metadatas) {
			if (tmp == -1 && metadata.type != NON_EXISTENT) {
				tmp = metadata.mtime;
				continue;
			}
			if (metadata.type != NON_EXISTENT && metadata.mtime != tmp && metadata.type != DIRECTORY) {
				base::to_be_synced += file.metadatas.size() - 1 - files_dont_exist;
				break;
			} else if (metadata.type == NON_EXISTENT) {
				base::to_be_synced++;
				files_dont_exist++;
			}
		}
	}
}

#ifdef REMOTE_ENABLED
void free_rsessions() {
	for (auto& input_path : input_paths) {
		if (input_path.remote == false)
			continue;
		ssh_session ssh = input_path.sftp->session;
		rsession::free_sftp(input_path.sftp);
		rsession::free_ssh(ssh);
	}
}
#endif // REMOTE_ENABLED

void diff_input_paths(void) {
	auto print_and_exit = [&](const std::string& path1, const std::string& path2) {
		llog::error("input paths are the same");
		llog::error(path1);
		llog::error(path2);
		llog::error("exiting...");
		exit(1);
	};

	size_t out = 0, in = 0;
	for (const auto& path1 : input_paths) {
		in = 0;
		for (const auto& path2 : input_paths) {
			if (out == in)
				goto end;
			else if ((path1.remote && !path2.remote) || (!path1.remote && path2.remote))
				goto end;

			if (!path1.remote && !path2.remote && path1.path == path2.path)
				print_and_exit(path1.path, path2.path);
#ifdef REMOTE_ENABLED
			else if (path1.remote && path2.remote && path1.path == path2.path) {
				std::string hostname1 = path1.ip.substr(0, path1.ip.find(":"));
				std::string hostname2 = path2.ip.substr(0, path2.ip.find(":"));
				if (hostname1 == hostname2) {
					print_and_exit(path1.ip, path2.ip);
				}
			}
#endif // REMOTE_ENABLED
		end:
			in++;
		}
		out++;
	}
}

void print_stats() {
	llog::print("");
	for (const auto& input_path : input_paths) {
		std::string stats = size_units(input_path.synced_size);
		stats += ", Files: " + std::to_string(input_path.synced_files);
		stats += ", Dirs: " + std::to_string(input_path.synced_dirs);
		if (options::remove_extra) {
			stats += ", Removed dirs: " + std::to_string(input_path.removed_dirs);
			stats += ", Removed files: " + std::to_string(input_path.removed_files);
			stats += ", Freed : " + size_units(input_path.removed_size);
		}
		llog::print("total synced to '" + input_path.path + "': " + stats);
	}
}

int init_program(void) {
	base::paths_count = input_paths.size();
#ifdef REMOTE_ENABLED
	init_rsessions();
#endif // REMOTE_ENABLED
	diff_input_paths();
	fill_base();
	counter();
	if (options::resume && !part_files.empty())
		resume::init();
	else
		options::resume = false;
	syncing();
	if (options::remove_extra)
		remove_extra();
#ifdef REMOTE_ENABLED
	free_rsessions();
#endif // REMOTE_ENABLED
	print_stats();
	return 0;
}
