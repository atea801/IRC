#include "message.hpp"

const std::string &message::get_command() const
{
    return command;
}

const std::vector<std::string> &message::get_args() const
{
    return args;
}