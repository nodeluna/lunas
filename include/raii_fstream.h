#ifndef RAII_FSTREAM
#define RAII_FSTREAM

#include <fstream>
#include <string>

namespace raii {
	namespace fstream {
		class file {
				std::fstream* _file;
				std::string   _path;

			public:
				explicit file(std::fstream* file, const std::string& path);
				~file();
		};
	}

}

#endif // RAII_FSTREAM
