#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <vector>
#include <string>
#include <string_view>

//stolen from stackoverflow
std::vector<std::string_view> split(const std::string_view& s, const std::string_view& delimiter);

int to_int(const std::string_view & input);

float to_float(const std::string_view &input);

std::string join(const std::vector<std::string_view>& array, const std::string_view& join_string);

std::string remove_whitespace(std::string str);
#endif
