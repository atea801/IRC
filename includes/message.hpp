#pragma once

#include <string>
#include <vector>

class message {
    private :
        std::string command;
        std::vector<std::string> args;
    public :
    int message_objects(std::string data);
    const std::string &get_command() const;
    const std::vector<std::string> &get_args() const;
};