#include "Server.hpp"

void Server::exec_flow(Message &msg, Client &c)
{
	std::string cmd = msg.get_command();
	for (size_t i = 0; i < cmd.size(); i++)
        cmd[i] = toupper(cmd[i]);
    //add accept lower case
	if (c.getStatus() != REGISTERED){
		if (cmd != "PASS" && cmd != "NICK" && cmd != "USER"){
			send_reply_error(c, ERR_NOTREGISTERED, "You have not registered");
		}
	}
    if (cmd == "CAP")
        handle_cap(c);
    else if (cmd == "PING")
        handle_ping(c);
    else if (cmd == "PASS")
        handle_pass(msg, c);
    else if (cmd == "NICK")
        handle_nick(msg, c);
    else if (cmd == "USER")
        handle_user(msg, c);
    else if (msg.get_command() == "QUIT")
        c.setStatus(QUIT);
    else if (msg.get_command() == "PRIVMSG")
        handle_privmsg(msg, c);
}

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
    //check prealable
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
    //execution final
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
    const std::vector<std::string> args = msg.get_args();
    if (args.size() != 1)
    {
        error = ERR_NEEDMOREPARAMS;
        return;
    }
    if (args[0] != this->password)
    {
        error = ERR_PASSWDMISMATCH;
		send_reply_error(c, error, "Password incorrect");
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

void Server::handle_cap(Client &c)
{
    std::string reply = "CAP * LS :\r\n";
    send(c.getFdClient(), reply.c_str(), reply.size(), 0);
}

void Server::handle_ping(Client &c)
{
    std::vector<std::string> args;
    if (args.empty() || args.size() != 1 || args[0].empty())
        return;
    std::string msg_to_send = "PONG :" + args[0];
    send(c.getFdClient(), msg_to_send.c_str(), msg_to_send.size(), 0);
}
