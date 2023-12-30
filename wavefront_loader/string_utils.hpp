#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <vector>
#include <string>
#include <cstring>

//stolen from stackoverflow
std::vector<std::string> split(const std::string& s, const std::string& delimiter);

std::string join(const std::vector<std::string>& array, const std::string& join_string);
#endif
