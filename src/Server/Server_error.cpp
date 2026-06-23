#include "Server.hpp"

void Server::send_reply_error(Client &c, IrcError error, const std::string &message){
	
	int code = static_cast<int>(error);
	std::ostringstream reply;

	reply << ":irc.server " 
          << code
          << " " << c.getNickname() 
          << " :" << message << "\r\n";
    send(c.getFdClient(), reply.str().c_str(), reply.str().size(), 0);
}