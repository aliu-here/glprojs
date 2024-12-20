#include <algorithm>
#include <vector>
#include <string>
#include <string_view>
#include <charconv>
#include <stdexcept>

//stolen from stackoverflow
std::vector<std::string_view> split(const std::string_view &s, const std::string_view& delimiter) 
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string_view token;
    std::vector<std::string_view> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }
    res.push_back (s.substr (pos_start));

    return res;
}

int to_int(const std::string_view & input)
{
    int out;
    const std::from_chars_result result = std::from_chars(input.data(), input.data() + input.size(), out);
    if(result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range)
    {
        throw (std::invalid_argument("conversion failed"));
    }
    return out;
}

float to_float(const std::string_view &input)
{
    float out;
    const std::from_chars_result result = std::from_chars(input.data(), input.data() + input.size(), out);
    if(result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range)
    {
        throw (std::invalid_argument("conversion failed"));
    }
    return out;
}

std::string join(const std::vector<std::string_view>& array, const std::string_view& join_string)
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

std::string remove_whitespace(std::string& str)
{
    std::string copy = str;
    copy.erase(std::remove_if(copy.begin(), copy.end(), ::isspace), copy.end());
    return str;
}
