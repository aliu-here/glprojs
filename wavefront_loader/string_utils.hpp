#include <vector>
#include <string>
#include <cstring>

#ifndef BASIC_STRING_OPS
#define BASIC_STRING_OPS
//stolen from stackoverflow
std::vector<std::string> split(const std::string& s, const std::string& delimiter);

std::string join(const std::vector<std::string>& array, const std::string& join_string);
#endif
