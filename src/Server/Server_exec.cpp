#include "Server.hpp"

void Server::exec_flow(Message &msg, Client &c)
{
    std::string cmd = msg.get_command();
    for (size_t i = 0; i < cmd.size(); i++)
        cmd[i] = toupper(cmd[i]);
    if (cmd.empty())
        return;
    // std::cout << "[cmd]=[" << cmd << "]" << std::endl;
    if (c.getStatus() != REGISTERED)
    {
        if (cmd != "PASS" && cmd != "NICK" && cmd != "USER")
        {
            send_reply_error(c, ERR_NOTREGISTERED, "You have not registered");
            return;
        }
    }

    if (cmd == "PASS")
        handle_pass(msg, c);
    else if (cmd == "NICK")
        handle_nick(msg, c);
    else if (cmd == "USER")
        handle_user(msg, c);
    else if (cmd == "QUIT")
        c.setStatus(QUIT);
    else if (cmd == "PRIVMSG")
        handle_privmsg(msg, c);
	else if (cmd == "JOIN")
		handle_join(msg, c);
    else
        send_reply_error(c, ERR_UNKNOWNCOMMAND, cmd, "Unknown command");
    if (c.getBoolPass() && c.getBoolNick() && c.getBoolUser() && c.getStatus() != REGISTERED)
    {
         c.setStatus(REGISTERED);
    	const std::string &nick = c.getNickname();
    	send_raw(c, ":irc.server 001 " + nick + " :Welcome to the IRC Network " + nick);
    }
}

void Server::handle_nick(Message &msg, Client &c)
{
    IrcError error = msg.parsing_nick();
    if (error != IRC_OK)
    {
        if (error == ERR_NONICKNAMEGIVEN)
            send_reply_error(c, error, "No nickname given");
        else if (error == ERR_ERRONEUSNICKNAME)
            send_reply_error(c, error, msg.get_args()[0], "Erroneus nickname");
        return;
    }
    // check prealable
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
    // execution finale
    c.setNickname(args[0]);
    c.setBoolNick(true);
}

void Server::handle_user(Message &msg, Client &c)
{
    IrcError error = msg.parsing_user();
    if (error != IRC_OK)
    {
        if (error == ERR_NEEDMOREPARAMS)
            send_reply_error(c, error, "USER :Not enough parameters");
        if (error == ERR_INVALID)
            send_reply_error(c, error, "User is invalid");
        return;
    }
    std::vector<std::string> args = msg.get_args();
    // std::cerr << "args.size() = " << args.size() << std::endl;
    // for (size_t i = 0; i < args.size(); ++i)
    //     std::cerr << "  args[" << i << "] = [" << args[i] << "]" << std::endl;
    // ----------------------------------------------------

	if (c.getStatus() == REGISTERED)
    {
        send_reply_error(c, ERR_ALREADYREGISTERED, "You may not reregister");
        return;
    }

    IrcError error = msg.parsing_user();
    if (error != IRC_OK)
    {
        if (error == ERR_NEEDMOREPARAMS)
            send_reply_error(c, error, msg.get_command(), "Not enough parameters");
        return;
    }
    c.setUsername(args[0]);
    // parsing_user vérifie args.size() != 4 mais si le trailing est dans args[3]
    // — est-ce que ton parser met bien le realname en args[3] ? Vérifie avec un cerr de debug.
    c.setRealname(args[3]);
    c.setBoolUser(true);
}

void Server::handle_pass(Message &msg, Client &c)
{
	if (c.getStatus() == REGISTERED)
    {
        send_reply_error(c, ERR_ALREADYREGISTERED, "You may not reregister");
        return;
    }

    IrcError error = msg.parsing_pass();
    if (error != IRC_OK)
    {
        if (error == ERR_NEEDMOREPARAMS)
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
    if (args.empty() && args[0].empty())
        return;
    int dest = find_dest(args[0]);
    if (dest >= 0)
    {
        for (size_t i = 0; i < args.size(); i++)
            std::cout << "args[" << i << "] = [" << args[i] << "]" << std::endl;
        std::string msg_to_send = ":" + c.getNickname() + " PRIVMSG " + args[0] + " :" + args[1] + "\r\n";
        send(dest, msg_to_send.c_str(), msg_to_send.size(), 0);
    }
    else if (findChannelByName(args[0]))
    {
        Channel *Chan_to_send = findChannelByName(args[0]);
            std::string msg_to_send =
                ":" + c.getNickname() + " PRIVMSG " + args[0] + " :" + args[1];
            broadcastToChannel(*Chan_to_send, msg_to_send);
    }
    return;
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

    // le cas channelsRaw.size() == 0 est checké dans parsing_Kick()
    //  if channelsRaw.size() > 1 --> pas de numeric reply dédié. Claude:
    // La plupart des serveurs IRC renvoient ERR_NOSUCHCHANNEL car ils
    // prennent l'ensemble de l'arg des channels comme un nom de channel
    //  unique (par ex #a,&b)
    if (channelsRaw.size() > 1 || checkChannels(channelsRaw) == ERR_NOSUCHCHANNEL)
    {
        // ERR_NOSUCHCHANNEL (403)
        // send_reply_error "<client> <channel> :No such channel"
        return;
    }
    Channel *chan = findChannelByName(channelsRaw[0]);
    if (!chan->isMember(c))
    {
        // ERR_NOTONCHANNEL (442)
        // send_reply_error "<client> <channel> :You're not on that channel"
        return;
    }
    if (!chan->isOperator(c))
    {
        // ERR_CHANOPRIVSNEEDED (482)
        // send_reply_error "<client> <channel> :You're not channel operator"
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
            // ERR_USERNOTINCHANNEL (441) "<client> <nick> <channel> :They aren't on that channel"
            continue;
        }

        std::string line = ":" + kickerPrefix + " KICK " + chan->getName() + " " + clientsRaw[i] + " :" + comment;
        broadcastToChannel(*chan, line); // envoyé à tous, y compris la cible, AVANT le retrait

        chan->removeMember(*target);
        if (chan->isOperator(*target))
            chan->removeOperator(*target);
    }
}

int Server::find_dest(std::string dest)
{
    if (dest[0] != '#')
    {
        for (size_t i = 0; i < vec_clients.size(); i++)
        {
            if (vec_clients[i].getNickname() == dest)
                return vec_clients[i].getFdClient();
        }
    }
    return (-1);
}

int Server::find_channel_index(std::string dest)
{
    if (channels.size() > 0)
    {
        for (size_t i = 0; i < channels.size(); i++)
        {
            if (channels[i].getName() == dest)
                return (i);
        }
    }
    return (-1);
}

void Server::handle_cap(Client &c)
{
    std::string reply = "CAP * LS :\r\n";
    send(c.getFdClient(), reply.c_str(), reply.size(), 0);
}

void Server::handle_ping(Message &msg, Client &c)
{
    std::vector<std::string> args = msg.get_args();
    if (args.empty() || args.size() != 1 || args[0].empty())
        return;
    std::string msg_to_send = "PONG :" + args[0] + "\r\n";
    send(c.getFdClient(), msg_to_send.c_str(), msg_to_send.size(), 0);
}

void Server::remove_client(int fd)
{
    vec_clients.erase(fd);
}

// fonction en cours de creation
void Server::handle_quit(Message &msg, Client &c)
{
    std::vector<std::string> args = msg.get_args();
    std::string reason = (!args.empty() && !args[0].empty()) ? args[0] : "Client Quit";
    std::string msg_to_send = ":" + c.getNickname() + "!" + c.getUsername()
                     + "@localhost QUIT :" + reason;
    PtrVec<Channel> c_channels = c.get_client_channel();
    for (size_t i = 0; i < c_channels.size(); i++)
    {
        Channel *chan = c_channels.get()[i];
        broadcastToChannel(*chan, msg_to_send, &c);
        chan->removeMember(c);
        if (chan->isOperator(c))
            chan->removeOperator(c);
    }
    c.setStatus(QUIT);
}

void Server::handle_join(Message &msg, Client &c)
{
    // IrcError error = msg.parsing_join();
    // if (error != IRC_OK)
    // {}
    std::vector<std::string> args = msg.get_args();
    if (!findChannelByName(args[0]))
    {
        Channel new_channel;
        new_channel.setName(args[0]);
        new_channel.addMember(c);
        new_channel.addOperator(c);
        channels.push_back(new_channel);
        c.addChannel(new_channel);
        std::cout << "channel as been created\n";
    }
    else
    {
        Channel *ChanToJoin = findChannelByName(args[0]);
        ChanToJoin->addMember(c);
        c.addChannel(*ChanToJoin);
        // :nick!user@localhost JOIN #channel
        std::string msg_to_send = ":" + c.getNickname() + "!" + c.getUsername()
                     + "@localhost JOIN :" + ChanToJoin->getName();
        broadcastToChannel(*ChanToJoin, msg_to_send);
    }
}
