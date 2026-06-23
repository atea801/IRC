#include "Server.hpp"

void Server::execute(Message &msg, Client &c)
{
    if (msg.get_command() == "PASS")
        handle_pass(msg, c);
    else if (msg.get_command() == "NICK")
        handle_nick(msg, c);
    else if (msg.get_command() == "USER")
        handle_user(msg, c);
    else if (msg.get_command() == "QUIT")
        c.setStatus(QUIT);
    else if (msg.get_command() == "PRIVMSG")
        handle_privmsg(msg, c);
    if (c.getBoolPass() == true && c.getBoolUser() == true && c.getBoolNick() == true)
        c.setStatus(REGISTERED);
}

void Server::handle_nick(Message &msg, Client &c)
{
    IrcError error = msg.parsing_nick();
    if (error != IRC_OK)
    {
    }
    std::vector<std::string> args = msg.get_args();
    for (size_t i = 0; i < vec_clients.size(); i++)
    {
        if (&vec_clients[i] != &c && vec_clients[i].getNickname() == args[0])
        {
            error = ERR_NICK_TAKEN;
            return;
        }
    }
    c.setNickname(args[0]);
    c.setBoolNick(true);
}

void Server::handle_user(Message &msg, Client &c)
{
    IrcError error = msg.parsing_user();
    if (error != IRC_OK)
    {
    }
    std::vector<std::string> args = msg.get_args();
    c.setUsername(args[0]);
    c.setRealname(args[3]);
    c.setBoolUser(true);
}

void Server::handle_pass(Message &msg, Client &c)
{
    IrcError error = msg.parsing_pass();
    if (error != IRC_OK)
    {
    }
    const std::vector<std::string> args = msg.get_args();
    if (args.size() != 1)
    {
        error = ERR_NEEDMOREPARAMS;
        return;
    }
    if (args[0] != this->password)
    {
        error = ERR_PASSWDMISMATCH;
        return;
    }
    c.setBoolPass(true);
}

void Server::handle_privmsg(Message &msg, Client &c)
{
    // IrcError error = msg.parsing_privmsg();
    // if (error != IRC_OK)
    // {

    // }
    const std::vector<std::string> args = msg.get_args();
    int dest = find_dest(args[0]);
    std::cout << "ARGS SIZE = " << args.size() << std::endl;
    for (size_t i = 0; i < args.size(); i++)
        std::cout << "args[" << i << "] = [" << args[i] << "]" << std::endl;
    std::string msg_to_send = ":" + c.getNickname() + " PRIVMSG " + args[0] + " :" + args[1] + "\r\n";
    send(dest, msg_to_send.c_str(), msg_to_send.size(), 0);
}

int Server::find_dest(std::string dest)
{
    for (size_t i = 0; i < vec_clients.size(); i++)
    {
        if (vec_clients[i].getNickname() == dest)
            return vec_clients[i].getFdClient();
    }
    return -1;
}

