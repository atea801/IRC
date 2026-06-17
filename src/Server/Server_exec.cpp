#include "Server.hpp"

void Server::execute(Message &msg, Client &c)
{
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
