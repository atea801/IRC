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
		if(error == ERR_ERRONEUSNICKNAME)
			send_reply_error(c, error, msg.get_args()[0], "Erroneus nickname");
		return;
    }
    //check prealable
    std::vector<std::string> args = msg.get_args(); 
    for (size_t i = 0; i < vec_clients.size(); i++)
    {
        if (&vec_clients[i] != &c && vec_clients[i].getNickname() == args[0])
        {
            error = ERR_NICKNAMEINUSE;
			send_reply_error(c, error, msg.get_args()[0], "Nickname is already in use");
            return;
        }
    }
    //execution finale
    c.setNickname(args[0]);
    c.setBoolNick(true);
}

void Server::handle_user(Message &msg, Client &c)
{
    IrcError error = msg.parsing_user();
    if (error != IRC_OK){
		if(error == ERR_NEEDMOREPARAMS)
			send_reply_error(c, error, msg.get_command(), "Not enough parameters");
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
			send_reply_error(c, error, msg.get_command(), "Not enough parameters");
		return;
    }
    const std::vector<std::string> args = msg.get_args();
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
ERR_NEEDMOREPARAMS (461) ok
    ERR_NOSUCHCHANNEL (403) ok
    ERR_CHANOPRIVSNEEDED (482) ok
    ERR_USERNOTINCHANNEL (441)
    ERR_NOTONCHANNEL (442) ok
*/
void Server::handle_Kick(Message &msg, Client &c)
{
    /*IrcError error = msg.parsing_Kick();
    if (error != IRC_OK)
    {
		if(error == ERR_NEEDMOREPARAMS)
			send_reply_error(c, error, " KICK :Not enough parameters");
		return;
    }
    */

    std::vector<std::string> channelsRaw = findChannelsInMsg(msg);

    //le cas channelsRaw.size() == 0 est checké dans parsing_Kick()
    // if channelsRaw.size() > 1 --> pas de numeric reply dédié. Claude: 
    //La plupart des serveurs IRC renvoient ERR_NOSUCHCHANNEL car ils 
    //prennent l'ensemble de l'arg des channels comme un nom de channel
    // unique (par ex #a,&b) 
    if (channelsRaw.size() > 1 || checkChannels(channelsRaw) == ERR_NOSUCHCHANNEL)
    {
        //ERR_NOSUCHCHANNEL (403)
        //send_reply_error "<client> <channel> :No such channel"
    	return;
    }
    Channel *chan = findChannelByName(channelsRaw[0]);
    if (!chan->isMember(c))
    {
        //ERR_NOTONCHANNEL (442)
        //send_reply_error "<client> <channel> :You're not on that channel"
    	return;
    }
    if (!chan->isOperator(c))
    {
        //ERR_CHANOPRIVSNEEDED (482)
        //send_reply_error "<client> <channel> :You're not channel operator"
    	return;
    }

    std::vector<std::string> clientsRaw = ft_split(',', msg.get_args()[1]);

    // Préfixe de l'émetteur (nick!user@host) et commentaire optionnel (défaut: nick du kicker)
    std::string kickerPrefix = c.getNickname() + "!" + c.getUsername() + "@" + c.getHostname();
    std::string comment = (msg.get_args().size() > 2) ? msg.get_args()[2] : c.getNickname();

    // Un message KICK distinct par utilisateur, diffusé à tout le channel.
    // Une fois le message envoyé, on supprime le client qui doit être Kick
    for (size_t i = 0; i < clientsRaw.size(); i++)
    {
        Client *target = findClientByNickname(clientsRaw[i]);
        if (target == NULL || !chan->isMember(*target))
        {
            //ERR_USERNOTINCHANNEL (441) "<client> <nick> <channel> :They aren't on that channel"
            continue;
        }

        std::string line = ":" + kickerPrefix + " KICK " + chan->getName() + " " + clientsRaw[i] + " :" + comment;
        broadcastToChannel(*chan, line); // envoyé à tous, y compris la cible, AVANT le retrait

        chan->removeMember(*target);
        if (chan->isOperator(*target))
            chan->removeOperator(*target);
    }
}

/*
Erreurs de parsing: ERR_NEEDMOREPARAMS, ERR_UNKNOWNMODE
*/
void Server::handle_mode(Message &msg, Client &c)
{
    /*IrcError error = msg.parsing_mode();
    if (error != IRC_OK)
    {
		if(error == ERR_NEEDMOREPARAMS)
			send_reply_error(c, error, " MODE :Not enough parameters");
        if(error == ERR_UNKNOWNMODE)
			send_reply_error "<client> <modechar> :is unknown mode char to me"
		return;
    }
    */

    Channel * chan = findChannelByName(msg.get_args()[0]);
    if (!chan)
    {
        //ERR_NOSUCHCHANNEL (403)
        //send_reply_error "<client> <channel> :No such channel"
    	return;
    }
    if (msg.get_args().size() == 1) //MODE #general --> demande les modes activés
    {
        //RPL_CHANNELMODEIS (324)
        //"<client> <channel> <modestring> <mode arguments>..."
        //exemple avec tous les modes possibles: ":irc.42.fr 324 dan #music +itkl secret 42"
        //secret est le password (+k)
        //42 est le user limit (+l)
        //i et t ne donnent pas de mode arguments
        //Attention: le mode +o est exclu de ce num reply
    }
    if (!chan->isOperator(c))
    {
        //ERR_CHANOPRIVSNEEDED (482)
        //send_reply_error "<client> <channel> :You're not channel operator"
    	return;
    }
    std::string modestring = msg.get_args()[1];
    std::vector<std::string> mode_args;
    const std::vector<std::string> &args = msg.get_args();
    
    for (size_t i = 2; i < args.size(); i++)
        mode_args.push_back(args[i]);

    std::string::iterator it = modestring.begin();
    char sign;
    size_t args_idx = 0;
    while (it != modestring.end())
    {
        if (*it == '+' || *it == '-')
        {
            sign = *it;
            it++;
        }
        if (*it == 'o')
        {
            if (findClientByNickname(mode_args[args_idx]) == NULL)
            {
                //ERR_USERNOTINCHANNEL (441) "<client> <nick> <channel> :They aren't on that channel"
                args_idx++; 
                //PAS DE return car on continue l'exec des autres modes demandés 
                //on peut avoir par ex "MODE +ok-i Bob secret" et meme si Bob ne fait pas partie du channel
                //on doit continuer à traiter les autres modes
            }
        }
        std::string param;
        if (modeNeedsParam(sign, *it))
            param = mode_args[args_idx++]; //pas de check si mode_args contient bien des args car fait au parsing
        identify_and_exec_mode(*chan, c, sign, *it, param);
        it++;
    }
}

void identify_and_exec_mode(Channel &chan, Client &c, char sign, char mode_letter, const std::string &param)
{
    bool set;
    if (sign == '+')
        set = true;
    else
        set = false;
    switch (mode_letter)
    {
        case 'i':
        {
            chan.setInviteOnly(set);
            break;
        }
        case 't':
        {
            chan.setTopicRestricted(set);
            break;
        }
        case 'k':
        {
            if (set == true)
                chan.setPassword(param);
            else
                chan.removePassword();
            break;
        }
        case 'o':
        {
            if (set == true)
                chan.addOperator(c);
            else
                chan.removeOperator(c);
            break;
        }
        case 'l':
        {
            if (set == true)
                chan.setUserLimit(std::atoi(param.c_str()));
            else
                chan.removeUserLimit();
            break;
        }
    }
}

// Type B (k, o): param on '+' AND '-'. Type C (l): param only on '+'. Type D (i, t): never.
static bool modeNeedsParam(char sign, char letter)
{
    if (letter == 'k' || letter == 'o') return true;
    if (letter == 'l')                  return (sign == '+');
    return false;
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

int Server::find_channel_index(std::string dest)
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