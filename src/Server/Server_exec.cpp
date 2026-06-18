#include "Server.hpp"

void Server::execute(Message &msg, Client &c)
{
    handle_nick(msg, c);

}

void Server::handle_nick(Message &msg, Client &c)
{
    IrcError error = msg.parsing_nick();
    if (error != IRC_OK)
    {
    }
    std::vector<std::string> args = msg.get_args();
    for (int i = 0; i < vec_clients.size(); i++)
    {
        if (&vec_clients[i] != &c && vec_clients[i].getNickname() == args[0])
        {
            error = ERR_NICK_TAKEN;
            return;
        }
    }
    c.setNickname(args[0]);
}

void Server::handle_user(Message &msg, Client &c)
{
    IrcError error = msg.parsing_user();
    if (error != IRC_OK)
    {
    }
    std::vector<std::string> args = msg.get_args();
    for (int i = 0; i < vec_clients.size(); i++)
    {
        if (&vec_clients[i] != &c && vec_clients[i].getUsername() == args[0])
        {
            error = ERR_USER_TAKEN;
            return;
        }
    }
    c.setUsername(args[0]);
}

void Server::handle_user(Message &msg, Client &c)
{
    IrcError error = msg.parsing_realname();
    if (error != IRC_OK)
    {
    }
    std::vector<std::string> args = msg.get_args();
    for (int i = 0; i < vec_clients.size(); i++)
    {
        if (&vec_clients[i] != &c && vec_clients[i].getRealname() == args[0])
        {
            error = ERR_REALNAME_TAKEN;
            return;
        }
    }
    c.setRealname(args[0]);
}

void Server::handle_pass(Message &msg, Client &c)
{
    IrcError error = msg.parsing_pass();
    if (error != IRC_OK)
    {
    }
    const std::vector<std::string>& args = msg.get_args();
    if (args.size() != 1)
        return ERR_NEEDMOREPARAMS;

    if (args[0] != this->password)
        return ERR_PASSWDMISMATCH;

    c.setAuthenticated(true);
}


