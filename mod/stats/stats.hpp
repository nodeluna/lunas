#pragma once

#include <string>
#include <string>
#include <cstdint>

#include "input_path/input_path.hpp"

namespace lunas
{
	std::string decimal_precision(const double& number, const int& percision);
	std::string size_units(const std::uintmax_t& bytes);
	void	    print_stats(const struct lunas::parsed_data& data);
}
