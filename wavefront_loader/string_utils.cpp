#include "string_utils.hpp"
//stolen from stackoverflow
std::vector<std::string> split(const std::string& s, const std::string& delimiter) 
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

std::string join(const std::vector<std::string>& array, const std::string& join_string)
{
        std::string output;
        int i=0;
        for (;i<array.size() - 1; i++)
        {
                output.append(array[i]);
                output.append(join_string);
        }
        output.append(array[i]);
        return output;
}
