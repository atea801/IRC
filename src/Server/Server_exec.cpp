#include "Server.hpp"

void Server::exec_flow(Message &msg, Client &c)
{
    std::string cmd = msg.get_command();
    for (size_t i = 0; i < cmd.size(); i++)
        cmd[i] = toupper(cmd[i]);
    if (cmd.empty())
        return;
    if (c.getStatus() != REGISTERED)
    {
        if (cmd != "PASS" && cmd != "NICK" && cmd != "USER" && cmd != "CAP")
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
        handle_quit(msg, c);
    else if (cmd == "PRIVMSG")
        handle_privmsg(msg, c);
    else if (cmd == "JOIN")
        handle_join(msg, c);
    else if (cmd == "MODE")
        handle_mode(msg, c);
    else if (cmd == "KICK")
        handle_kick(msg, c);
    else if (cmd == "TOPIC")
        handle_topic(msg, c);
    else if (cmd == "INVITE")
        handle_invite(msg, c);
    else if (cmd == "PART")
        handle_part(msg, c);
    else if (cmd == "PING")
        handle_ping(msg, c);
    else if (cmd == "CAP")
        handle_cap(msg, c);
    else
        send_reply_error(c, ERR_UNKNOWNCOMMAND, cmd, "Unknown command");
    if (c.getBoolPass() && c.getBoolNick() && c.getBoolUser() && c.getStatus() != REGISTERED)
    {
        c.setStatus(REGISTERED);
        const std::string &nick = c.getNickname();
        send_raw(c, ":irc.server 001 " + nick + " :Welcome to the IRC Network " + nick);
        send_raw(c, ":" + _server_name + " 002 " + nick + " :Your host is " + _server_name + ", running version 1.0");
        send_raw(c, ":" + _server_name + " 003 " + nick + " :This server was created recently");
        send_raw(c, ":" + _server_name + " 004 " + nick + " " + _server_name + " 1.0 * itkol");
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
    std::string old_prefix = getPrefix(c);
    c.setNickname(args[0]);
    c.setBoolNick(true);
    //NICK <nick>
    std::string message = ":" + old_prefix + " NICK :" + args[0];
    send_raw(c, message);
    PtrVec<Channel> channels_list = c.get_client_channel();
    for (size_t i = 0; i < channels_list.size(); i++)
        broadcastToChannel(*channels_list.get()[i], message, &c); 
}

void Server::handle_user(Message &msg, Client &c)
{
    IrcError error = msg.parsing_user();
    if (error != IRC_OK)
    {
        if (error == ERR_NEEDMOREPARAMS)
            send_reply_error(c, error, msg.get_command(), "Not enough parameters");
        if (error == ERR_INVALID)
            send_reply_error(c, error, "User is invalid");
        return;
    }
    std::vector<std::string> args = msg.get_args();
    if (c.getStatus() == REGISTERED)
    {
        send_reply_error(c, ERR_ALREADYREGISTERED, "You may not reregister");
        return;
    }
    c.setUsername(args[0]);
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
    IrcError error = msg.parsing_privmsg();
    if (error != IRC_OK)
    {
        if (error == ERR_NORECIPIENT)
            send_reply_error(c, error, "No recipient given (PRIVMSG)");
        else if (error == ERR_NOTEXTTOSEND)
            send_reply_error(c, error, "No text to send");
        return;
    }

    const std::vector<std::string> args = msg.get_args();
    const std::vector<std::string> destinataires = ft_split(',', args[0]);

    // le texte ne change jamais selon la cible : c'est toujours args[1]
    const std::string &text = args[1];
    // prefixe complet nick!user@host, comme dans handle_Kick
    std::string prefix = getPrefix(c);
    // parcour des destinataires qu'il y en ai 1 ou 10
    for (size_t i = 0; i < destinataires.size(); i++)
    {
        const std::string &target = destinataires[i]; // LA cible du tour courant
        int find = find_dest(target);

        if (find >= 0) // c'est un nick connu
        {
            std::string msg_to_send = ":" + prefix + " PRIVMSG " + target + " :" + text + "\r\n";
            send(find, msg_to_send.c_str(), msg_to_send.size(), 0);
        }
        else if (findChannelByName(target)) // c'est un channel
        {
            Channel *chan = findChannelByName(target);
            if (chan->isMember(c) == true)
            {
                std::string msg_to_send = ":" + prefix + " PRIVMSG " + target + " :" + text;
                broadcastToChannel(*chan, msg_to_send, &c);
            }
            else
                send_reply_error(c, ERR_NOTONCHANNEL, "You are not a member of this channel");
        }
        else // ni nick ni channel
        {
            send_reply_error(c, ERR_NOSUCHNICK, target, "No such nick/channel");
            continue;
        }
    }
}

/*
Traitement de l'exec de KICK. Format: <channel> <user> *( "," <user> ) [<comment>]
Exemple : `KICK #chan Alice,Bob byeee`

Voir sur Figma le diagramme d'exec flow suivi par cette fonction.

Après check des numeric replies, un message KICK distinct par user est
diffusé à tout le channel, y compris le ou les users KICK, puis suppression du ou
des users du Channel.
*/
void Server::handle_kick(Message &msg, Client &c)
{
    IrcError error = msg.parsing_kick();
    if (error != IRC_OK)
    {
        send_reply_error(c, error, "KICK", "Not enough parameters");
        return;
    }
    std::vector<std::string> channelsRaw = findChannelsInMsg(msg);
    if (channelsRaw.empty())
    {
        send_reply_error(c, ERR_NEEDMOREPARAMS, "KICK", "Not enough parameters");
        return;
    }
    if (channelsRaw.size() > 1 || checkChannels(channelsRaw) == ERR_NOSUCHCHANNEL)
    {
        send_reply_error(c, ERR_NOSUCHCHANNEL, channelsRaw[0], "No such channel");
        return;
    }

    std::vector<std::string> clientsRaw = ft_split(',', msg.get_args()[1]);
    for (size_t i = 0; i < clientsRaw.size();)
    {
        if (clientsRaw[i].empty())
            clientsRaw.erase(clientsRaw.begin() + i);
        else
            i++;
    }
    if (clientsRaw.empty())
    {
        send_reply_error(c, ERR_NEEDMOREPARAMS, "KICK", "Not enough parameters");
        return;
    }

    Channel *chan = findChannelByName(channelsRaw[0]);
    if (!chan)
    {
        send_reply_error(c, ERR_NOSUCHCHANNEL, channelsRaw[0], "No such channel");
        return;
    }
    if (!chan->isMember(c))
    {
        send_reply_error(c, ERR_NOTONCHANNEL, chan->getName(), "You're not on that channel");
        return;
    }
    if (!chan->isOperator(c))
    {
        send_reply_error(c, ERR_CHANOPRIVSNEEDED, chan->getName(), "You're not channel operator");
        return;
    }

    // préparation du message KICK à envoyer:
    // on définit le préfixe de l'émetteur (nick!user@host) et le comment s'il a été fourni,
    // sinon comment par défaut: nick du kicker
    std::string kickerPrefix = getPrefix(c);
    std::string comment;
    if (msg.get_args().size() > 2 && !msg.get_args()[2].empty())
        comment = msg.get_args()[2];
    else
        comment = c.getNickname();

    // Un message KICK distinct par utilisateur, diffusé à tout le channel.
    // Une fois le message envoyé, on supprime le client qui doit être Kick
    for (size_t i = 0; i < clientsRaw.size(); i++) // boucle sur le nombre de users à Kick
    {
        Client *target = findClientByNickname(clientsRaw[i]);
        if (target == NULL || !chan->isMember(*target))
        {
            send_reply_error(c, ERR_USERNOTINCHANNEL, clientsRaw[i], chan->getName(), "They aren't on that channel");
            continue; // on continue à traiter les autres users à KICK
        }
        std::string line = ":" + kickerPrefix + " KICK " + chan->getName() + " " + clientsRaw[i] + " :" + comment;
        broadcastToChannel(*chan, line); // envoyé à tous, y compris la cible, AVANT le kick
        if (chan->isOperator(*target))
            chan->removeOperator(*target); // retiré AVANT removeMember
        chan->removeMember(*target);
    }
}

/*
Traitement de la commande TOPIC
Voir le diagramme d'exec flow sur Figma
Parameters: <channel> [<topic>]
*/
void Server::handle_topic(Message &msg, Client &c)
{
    IrcError error = msg.parsing_topic();
    if (error != IRC_OK)
    {
        send_reply_error(c, error, "TOPIC", "Not enough parameters");
        return;
    }

    Channel *chan = findChannelByName(msg.get_args()[0]);
    if (!chan)
    {
        //  "<client> <channel> :No such channel"
        send_reply_error(c, ERR_NOSUCHCHANNEL, msg.get_args()[0], "No such channel");
        return;
    }
    if (msg.get_args().size() == 1) // <topic> n'est pas fourni dans le Message
    {
        if (chan->getTopic() == "")
        {
            //send RPL_NOTOPIC
            //"<client> <channel> :No topic is set"
            send_reply_error(c, RPL_NOTOPIC, chan->getName(), "No topic is set");
            return;
        }
        else
        {
            //send RPL_TOPIC
            //"<client> <channel> :<topic>"
            send_reply_error(c, RPL_TOPIC, chan->getName(), chan->getTopic());
            return;
        }
    }
    if (chan->isTopicRestricted() && !chan->isOperator(c))
    {
        send_reply_error(c, ERR_CHANOPRIVSNEEDED, chan->getName(), "You're not channel operator");
        return;
    }
    std::string prefix = getPrefix(c);
    std::string msg_to_send = ":" + prefix + " ";
    if (msg.get_args()[1].empty()) //dans ce cas on efface le topic
    {
        chan->setTopic("");
        msg_to_send += "TOPIC " + chan->getName() + " :";
    }
    else
    {
        chan->setTopic(msg.get_args()[1]);
        msg_to_send += "TOPIC " + chan->getName() + " :" + msg.get_args()[1];
    }
    broadcastToChannel(*chan, msg_to_send, NULL); 
}

void Server::handle_cap(Message &msg, Client &c)
{
    if (msg.get_args().empty())
        return;
    const std::string &sub = msg.get_args()[0];

    if (sub == "LS" || sub == "LIST")
        send_raw(c, ":" + _server_name + " CAP * LS :");
    else if (sub == "REQ")
        send_raw(c, ":" + _server_name + " CAP * NAK :" + // on refuse tout
                        (msg.get_args().size() > 1 ? msg.get_args()[1] : ""));
    else if (sub == "END")
        return;
}

void Server::handle_ping(Message &msg, Client &c)
{
    std::vector<std::string> args = msg.get_args();
    if (args.empty() || args.size() != 1 || args[0].empty())
        return;
    std::string msg_to_send = "PONG :" + args[0] + "\r\n";
    send(c.getFdClient(), msg_to_send.c_str(), msg_to_send.size(), 0);
}

void Server::handle_quit(Message &msg, Client &c)
{
    std::vector<std::string> args = msg.get_args();
    std::string reason = (!args.empty() && !args[0].empty()) ? args[0] : "Client Quit";
    std::string msg_to_send = ":" + c.getNickname() + "!" + c.getUsername() + "@localhost QUIT :" + reason;
    PtrVec<Channel> c_channels = c.get_client_channel();
    for (size_t i = 0; i < c_channels.size(); i++)
    {
        Channel *chan = c_channels.get()[i];
        broadcastToChannel(*chan, msg_to_send, &c);
        chan->removeMember(c);
        if (chan->isOperator(c))
            chan->removeOperator(c);
        if (chan->getMembers().empty())
            remove_channel(chan->getName());
    }
    c.setStatus(QUIT);
}

void Server::handle_join(Message &msg, Client &c)
{
	IrcError error = msg.parsing_join();
	std::vector<std::string> args = msg.get_args();
	if (error != IRC_OK){
        if (error == ERR_NEEDMOREPARAMS)
            return (send_reply_error(c, error, msg.get_command(), "Not enough parameters"));
		else if (error == ERR_BADCHANMASK)
            return (send_reply_error(c, error, args[0], "Not enough parameters"));
	}

    Channel *chan = findChannelByName(args[0]);
    if (!chan)
    {
        Channel new_channel;
        new_channel.setName(args[0]);
        channels.insert(std::make_pair(args[0], new_channel));
        chan = findChannelByName(args[0]);
        chan->addMember(c);
        chan->addOperator(c);
        c.addChannel(*chan);
        std::string msg_to_send = ":" + c.getNickname() + "!" + c.getUsername() + "@localhost JOIN " + args[0];
        broadcastToChannel(*chan, msg_to_send);
        return;
    }
    if (chan->isMember(c))
        return;
    if (chan->isInviteOnly() && !chan->isInvited(c))
        return (send_reply_error(c, ERR_INVITEONLYCHAN, args[0],"Cannot join channel (+i)"));
    if (chan->hasPassword())
    {
        if (args.size() < 2 || args[1].empty())
            return (send_reply_error(c, ERR_BADCHANNELKEY, args[0], "Cannot join channel (+k)"));
        if (args[1] != chan->getPassword())
            return (send_reply_error(c, ERR_BADCHANNELKEY, args[0], "Cannot join channel (+k)"));
    }
    if (chan->hasUserLimit() && chan->NumberOfMembers() >= chan->getUserLimit())
        return (send_reply_error(c, ERR_CHANNELISFULL, args[0], "Cannot join channel (+l)"));
    chan->addMember(c);
    c.addChannel(*chan);

    //envoi du message JOIN à tous les membres du channel
    std::string msg_to_send = ":" + c.getNickname() + "!" + c.getUsername() + "@localhost JOIN " + args[0];
    broadcastToChannel(*chan, msg_to_send);
    
    // msg_to_send = "Welcome to " + c.getNickname() + " who has just join the channel *cheers*"; 
    //mis en comment car peut perturber le gestionnaire de client IRC

    //envoi du topic au nouveau membre si le topic est défini
    broadcastToChannel(*chan, msg_to_send, &c);
    if (chan->getTopic() != "")
        send_reply_error(c, RPL_TOPIC, chan->getName(), chan->getTopic());
    
    //envoi de la liste des membres du channel au nouveau membre (la liste inclut le nouveau membre)
    const std::vector<Client *> channelMembers = chan->getMembers();
    std::string membersList;
    for (size_t i = 0; i < channelMembers.size(); i++)
    {
        if (chan->isOperator(*channelMembers[i]))
            membersList += "@";
        membersList += channelMembers[i]->getNickname();
        if (i + 1 != channelMembers.size())
            membersList += " ";
    }
    send_reply_error(c, RPL_NAMREPLY, "= " + chan->getName(), membersList);
    send_reply_error(c, RPL_ENDOFNAMES, chan->getName(), "End of /NAMES list");
    // msg_to_send
    /*
    A list of users currently joined to the channel (with one or more RPL_NAMREPLY (353) numerics 
    followed by a single RPL_ENDOFNAMES (366) numeric). These RPL_NAMREPLY messages sent by the server 
    MUST include the requesting client that has just joined the channel.

    RPL_NAMREPLY
      "<client> <symbol> <channel> :[prefix]<nick>{ [prefix]<nick>}"
      <symbol> == "="

    RPL_ENDOFNAMES
        "<client> <channel> :End of /NAMES list"
    */
}

void Server::handle_part(Message &msg, Client &c)
{
    std::vector<std::string> args = msg.get_args();
    if (args.empty() || args[0].empty())
    {
        send_reply_error(c, ERR_NEEDMOREPARAMS, "PART", "Not enough parameters");
        return;
    }
    std::vector<std::string> channelsRaw = findChannelsInMsg(msg);
    if (channelsRaw.empty())
    {
        send_reply_error(c, ERR_NEEDMOREPARAMS, "PART", "Not enough parameters");
        return;
    }
    for (size_t i = 0; i < channelsRaw.size();)
    {
        if (channelsRaw[i].empty())
            channelsRaw.erase(channelsRaw.begin() + i);
        Channel *chan = findChannelByName(channelsRaw[i]);
        if (!chan)
        {
            send_reply_error(c, ERR_NOSUCHCHANNEL, channelsRaw[i], "No such channel");
            return;
        }
        if (!chan->isMember(c))
        {
            send_reply_error(c, ERR_NOTONCHANNEL, chan->getName(), "You're not on that channel");
            return;
        }
        else
        {
            if (args.size() > 1 && !args[1].empty())
            {
                std::string msg_to_send = ":" + c.getNickname() + "!" + c.getUsername() + "@localhost PART " +
                                          chan->getName() + ' ' + args[1];
                broadcastToChannel(*chan, msg_to_send);
            }
            else
            {
                std::string msg_to_send =
                    ":" + c.getNickname() + "!" + c.getUsername() + "@localhost PART " + chan->getName();
                broadcastToChannel(*chan, msg_to_send);
            }
            if (chan->isOperator(c))
                chan->removeOperator(c);
            chan->removeMember(c);
            c.removeChannel(*chan);
            if (chan->NumberOfMembers() == 0)
                channels.erase(chan->getName());
            i++;
        }
    }
}
