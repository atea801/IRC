#pragma once

#include "Channel.hpp"
#include "Client.hpp"
#include "irc_error.hpp"
#include "utils.hpp"
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
    void fill_cmd_and_args(size_t pos, std::string data);
    const std::string &get_command() const;
    const std::vector<std::string> &get_args() const;
    IrcError parsing_nick();
    IrcError parsing_user();
    IrcError parsing_pass();
    IrcError parsing_quit();
    IrcError parsing_privmsg();
    IrcError parsing_kick();
	IrcError parsing_invite();
	IrcError parsing_join();
};