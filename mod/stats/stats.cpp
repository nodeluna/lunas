module;

#if defined(IMPORT_STD_IS_SUPPORTED)
import std.compat;
#else
#	include <string>
#	include <string>
#	include <cstdint>
#	include <cmath>
#	include <print>
#endif

export module lunas.stats;
import lunas.ipath;

export namespace lunas {
	std::string decimal_precision(const double& number, const int& percision);
	std::string size_units(const std::uintmax_t& bytes);
	void	    print_stats(const struct lunas::parsed_data& data);
}

namespace lunas {
	std::string decimal_precision(const double& number, const int& percision) {
		const std::string temp = std::to_string(number);
		std::string	  decimal;
		if (temp.find('.') != temp.npos) {
			decimal = temp.substr(0, temp.find("."));
			decimal += temp.substr(temp.find('.'), percision + 1);
		} else
			decimal = temp;
		return decimal;
	}

	std::string size_units(const std::uintmax_t& bytes) {
		double	    exponent = std::log2(bytes);
		std::string output;
		if (exponent < 10)
			output = std::to_string(bytes) + " Bytes";
		else if (exponent < 20)
			output = decimal_precision(bytes / std::pow(1024, 1), 2) + " KiB";
		else if (exponent < 30)
			output = decimal_precision(bytes / std::pow(1024, 2), 2) + " MiB";
		else if (exponent < 40)
			output = decimal_precision(bytes / std::pow(1024, 3), 2) + " GiB";
		else if (exponent < 50)
			output = decimal_precision(bytes / std::pow(1024, 4), 2) + " TiB";
		else if (exponent < 60)
			output = decimal_precision(bytes / std::pow(1024, 5), 2) + " PiB";
		else if (exponent < 70)
			output = decimal_precision(bytes / std::pow(1024, 6), 2) + " EiB";
		else if (exponent < 80)
			output = decimal_precision(bytes / std::pow(1024, 7), 2) + " ZiB";
		else
			output = decimal_precision(bytes / std::pow(1024, 8), 2) + " YiB";
		return output;
	}

	void print_stats(const struct lunas::parsed_data& data) {
		std::println("");
		const auto& ipaths = data.get_ipaths();
		for (const auto& ipath : ipaths) {
			std::string str_stats = size_units(ipath.sync_stats.synced_size);
			str_stats += ", Files: " + std::to_string(ipath.sync_stats.synced_files);
			str_stats += ", Dirs: " + std::to_string(ipath.sync_stats.synced_dirs);
			if (data.options.remove_extra) {
				str_stats += ", Removed dirs: " + std::to_string(ipath.sync_stats.removed_dirs);
				str_stats += ", Removed files: " + std::to_string(ipath.sync_stats.removed_files);
				str_stats += ", Freed : " + size_units(ipath.sync_stats.removed_size);
			}

			std::println("total synced to '{}': {}", ipath.path, str_stats);
		}
	}
}
