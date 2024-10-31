#include <fstream>
#include <string>
#include "config.h"
#include "raii_fstream.h"
#include "log.h"

namespace raii {
	namespace fstream {
		file::file(std::fstream* file, const std::string& path) : _file(file), _path(path) {
		}

		file::~file() {
			if (_file->is_open() == false)
				return;

			_file->close();

			if (_file->is_open() == false)
				return;

			llog::local_error(_path, "couldn't close file", NO_EXIT);
		}
	}
}
