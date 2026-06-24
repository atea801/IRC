#include "Message.hpp"

const std::string &Message::get_command() const
{
    return command;
}

const std::vector<std::string> &Message::get_args() const
{
    return args;
}

const std::vector<std::string> &Message::get_channels() const
{
    return channels;
}

const std::vector<std::string> &Message::get_clients() const
{
    return clients;
}