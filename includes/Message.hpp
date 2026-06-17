#pragma once

#include "Client.hpp"
#include "irc_errors.hpp"
#include <string>
#include <vector>

class Message
{
  private:
    std::string command;
    std::vector<std::string> args;
    bool trailing_arg;

  public:
    Message();
    ~Message();
    Message(const Message &copy);
    Message &operator=(const Message &other);
    void clear();
    void extract_and_clean(Client &c);
    int parser_arg(Client &c);
    int handle_quit(std::vector<std::string> args);
    void fill_cmd_and_args(size_t pos, std::string data);
    const std::string &get_command() const;
    const std::vector<std::string> &get_args() const;
	NickError parsing_nick();
};