#include "Server.hpp"

void Server::execute(Message &msg, Client &c)
{
    if (msg.get_command() == "PASS")
        handle_pass(msg, c);
    else if (msg.get_command() == "NICK")
        handle_nick(msg, c);
    else if (msg.get_command() == "USER")
        handle_user(msg, c);
    if (c.getBoolPass() == true && c.getBoolUser() == true && c.getBoolNick() == true)
        c.setStatus(REGISTERED);
}

// int Message::handle_quit(std::vector<std::string> args)
// {
    
// }

void Server::handle_nick(Message &msg, Client &c)
{
    IrcError error = msg.parsing_nick();
    if (error != IRC_OK)
    {
		if(error == ERR_NONICKNAMEGIVEN)
			send_reply_error(c, error, "No nickname given");
		if(error == ERR_INVALID)
			send_reply_error(c, error, "No nickname is invalid");
		//sur ce message d'erreur normalement on met <client><nick> :message
		//comment faire remonter le <client> ? necessaire ?  detail
		if(error == ERR_ERRONEUSNICKNAME)
			send_reply_error(c, error, "Erroneus nickname");
		return;
    }
    std::vector<std::string> args = msg.get_args();
    for (size_t i = 0; i < vec_clients.size(); i++)
    {
        if (&vec_clients[i] != &c && vec_clients[i].getNickname() == args[0])
        {
            error = ERR_NICKNAMEINUSE;
			send_reply_error(c, error, "NICK: Nickname is already in use");
            return;
        }
    }
    c.setNickname(args[0]);
    c.setBoolNick(true);
}

void Server::handle_user(Message &msg, Client &c)
{
    IrcError error = msg.parsing_user();
    if (error != IRC_OK){
		if(error == ERR_NEEDMOREPARAMS)
			send_reply_error(c, error, "USER :Not enough parameters");
		if(error == ERR_INVALID)
			send_reply_error(c, error, "User is invalid");
		return;
    }
    std::vector<std::string> args = msg.get_args();
    c.setUsername(args[0]);
	//parsing_user vérifie args.size() != 4 mais si le trailing est dans args[3] 
	//— est-ce que ton parser met bien le realname en args[3] ? Vérifie avec un cerr de debug.
    c.setRealname(args[3]);
    c.setBoolUser(true);
}

void Server::handle_pass(Message &msg, Client &c)
{
    IrcError error = msg.parsing_pass();
    if (error != IRC_OK)
    {
		if(error == ERR_NEEDMOREPARAMS)
			send_reply_error(c, error, " PASS :Not enough parameters");
		return;
    }
    const std::vector<std::string> &args = msg.get_args();
    if (args[0] != this->password)
    {
        error = ERR_PASSWDMISMATCH;
		send_reply_error(c, error, "Password incorrect");
        return;
    }
    c.setBoolPass(true);
}
