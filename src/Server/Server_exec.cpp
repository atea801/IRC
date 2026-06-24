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
    if (cmd == "PASS")
        handle_pass(msg, c);
    else if (cmd == "NICK")
        handle_nick(msg, c);
    else if (cmd == "USER")
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

/*
Servers MUST NOT send multiple users in this message to clients, 
and MUST distribute these multiple-user KICK messages as a series of messages
with a single user name on each. This is necessary to maintain backward
compatibility with existing client software. 
If a KICK message is distributed in this way, <comment> (if it exists)
should be on each of these messages.
*/
void Server::handle_Kick(Message &msg, Client &c)
{
    /*IrcError error = msg.parsing_Kick();
    if (error != IRC_OK)
    {
		if(error == ERR_NEEDMOREPARAMS)
			send_reply_error(c, error, " KICK :Not enough parameters");
        if(error == ERR_NOSUCHCHANNEL)
            //send_reply_error();

        Numeric Replies (Source IRC Doc):

    ERR_NEEDMOREPARAMS (461)
    ERR_NOSUCHCHANNEL (403)
    ERR_CHANOPRIVSNEEDED (482)
    ERR_USERNOTINCHANNEL (441)
    ERR_NOTONCHANNEL (442)

		return;
    }
        L'erreur 461 est du parsing de syntaxe pure. 
        Toutes les autres erreurs nécessitent d'identifier la liste des 
        channels et la liste des clients dans msg.args, puis de vérifier
        si les channels et les clients existent vraiment, si le user à 
        l'origine de la commande KICK est bien un chanOps, etc. 
        Donc une fois que tous ces checks ont été faits, on peut en 
        récupérer un vecteur de pointeurs vers Channels et un vecteur de
        pointeurs vers Clients sur lesquels on peut gérer l'exec de KICK
    */
    
    // PtrVec<Channel> channelsFromMsg = get_channel_ptrs_from_message(msg);
    // PtrVec<Client> clientsFromMsg = get_client_ptrs_from_message(msg);

    PtrVec<Channel> channelsFromMsg;
    PtrVec<Client> clientsFromMsg;
    
    //si nb channels == nb clients alors on supprime le client i du channel i
    //vérifier si c'est bien la logique de KICK
    if (channelsFromMsg.size() == clientsFromMsg.size())
    {
        for (size_t i = 0; i < channelsFromMsg.size(); i++)
        {
            channelsFromMsg.get()[i]->removeMember(*clientsFromMsg.get()[i]);
            if (channelsFromMsg.get()[i]->isOperator(*clientsFromMsg.get()[i]))
                channelsFromMsg.get()[i]->removeOperator(*clientsFromMsg.get()[i]);
        }
    }
    else //sinon on supprime chaque user de chaque channel specifie
    //vérifier si c'est bien la logique de KICK
    {
        for (size_t chanIndex; chanIndex < channelsFromMsg.size(); chanIndex++)
        {
            for (size_t clientIndex = 0; clientIndex < clientsFromMsg.size(); clientIndex++)
            {
                channelsFromMsg.get()[chanIndex]->removeMember(*clientsFromMsg.get()[clientIndex]);
                if (channelsFromMsg.get()[chanIndex]->isOperator(*clientsFromMsg.get()[clientIndex]))
                    channelsFromMsg.get()[chanIndex]->removeOperator(*clientsFromMsg.get()[clientIndex]);
            }
        }
    }
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

int Server::find_channel(std::string dest)
{
    if (channels.size() > 0)
    {
        for (int i = 0; i < channels.size(); i++)
        {
            if (channels[i].getName() == dest)
                return (i);
        }
    }
    return (-1);
}