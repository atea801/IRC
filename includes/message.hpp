#pragma once

#include "Client.hpp"
#include <string>
#include <vector>

class message
{
  private:
    std::string command;
    std::vector<std::string> args;

  public:
    void extract_and_clean(Client &c);
    // int parser_cmd(std::string cmd);
    // int parser_arg(std::vector<std::string> args);
    void fill_cmd_and_args(size_t pos, std::string data);
    const std::string &get_command() const;
    const std::vector<std::string> &get_args() const;
};