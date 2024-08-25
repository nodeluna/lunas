#ifndef SIZE_UNITS
#define SIZE_UNITS

#include <string>
#include <cstdint>

std::string decimal_precision(const double& number, const int& percision);

std::string size_units(const std::uintmax_t& bytes);

#endif // SIZE_UNITS
