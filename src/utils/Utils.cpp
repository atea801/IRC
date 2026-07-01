#include "utils.hpp"

std::vector<std::string> ft_split(char separator, const std::string &str)
{
    size_t sep_idx;
    size_t pos;
    std::vector<std::string> result;

    pos = 0;
    sep_idx = str.find(separator, pos);
    while (sep_idx != std::string::npos)
    {
        result.push_back(str.substr(pos, sep_idx - pos));
        pos = sep_idx + 1; // skip the separator
        sep_idx = str.find(separator, pos);
    }
    result.push_back(str.substr(pos));
    return (result);
}
