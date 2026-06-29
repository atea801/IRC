#include "Server.hpp"

void Server::exec_flow(Message &msg, Client &c)
{
    std::string cmd = msg.get_command();
    for (size_t i = 0; i < cmd.size(); i++)
        cmd[i] = toupper(cmd[i]);
    // add accept lower case
    if (c.getStatus() != REGISTERED)
    {
        if (cmd != "PASS" && cmd != "NICK" && cmd != "USER")
        {
            send_reply_error(c, ERR_NOTREGISTERED, "You have not registered");
        }
    }
    if (cmd == "CAP")
        handle_cap(c);
    else if (cmd == "PING")
        handle_ping(msg, c);
    else if (cmd == "PASS")
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
        handle_mode(msg, c);
    if (c.getBoolPass() && c.getBoolNick() && c.getBoolUser() && c.getStatus() != REGISTERED)
    {
        c.setStatus(REGISTERED);
        std::string nick = c.getNickname();
        std::string reply = ":irc.server 001 " + nick + " :Welcome to the IRC Network " + nick + "\r\n";
        send(c.getFdClient(), reply.c_str(), reply.size(), 0);
    }
    std::cout << "REGCHECK pass=" << c.getBoolPass() << " nick=" << c.getBoolNick() << " user=" << c.getBoolUser()
              << " status=" << c.getStatus() << std::endl;
}

void Server::handle_nick(Message &msg, Client &c)
{
    IrcError error = msg.parsing_nick();
    if (error != IRC_OK)
    {
        if (error == ERR_NONICKNAMEGIVEN)
            send_reply_error(c, error, "No nickname given");
        if (error == ERR_INVALID)
            send_reply_error(c, error, "No nickname is invalid");
        // sur ce message d'erreur normalement on met <client><nick> :message
        // comment faire remonter le <client> ? necessaire ?  detail
        if (error == ERR_ERRONEUSNICKNAME)
            send_reply_error(c, error, "Erroneus nickname");
        return;
    }
    // check prealable
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
    // execution final
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
    c.setUsername(args[0]);
    // parsing_user vérifie args.size() != 4 mais si le trailing est dans args[3]
    // — est-ce que ton parser met bien le realname en args[3] ? Vérifie avec un cerr de debug.
    c.setRealname(args[3]);
    c.setBoolUser(true);
}

void Server::handle_pass(Message &msg, Client &c)
{
    IrcError error = msg.parsing_pass();
    if (error != IRC_OK)
    {
        if (error == ERR_NEEDMOREPARAMS)
            send_reply_error(c, error, " PASS :Not enough parameters");
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
        std::string msg_to_send = ":" + c.getNickname() + " PRIVMSG " + args[0] + " :" + args[1];
        broadcastToChannel(*Chan_to_send, msg_to_send);
    }
    return;
}

/*
Traitement de l'exec de KICK. Format: <channel> <user> *( "," <user> ) [<comment>]
Exemple : `KICK #chan Alice,Bob byeee`

Voir sur Figma le diagramme d'exec flow suivi par cette fonction.

Après check des numeric replies, un message KICK distinct par user est 
diffusé à tout le channel, y compris le ou les users KICK, puis suppression du ou 
des users du Channel.
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
    //  unique (par ex #a,&b --> nom de channel #a,&b)
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

    //préparation du message KICK à envoyer:
    //on définit le préfixe de l'émetteur (nick!user@host) et le comment s'il a été fourni,
    //sinon comment par défaut: nick du kicker
    std::string kickerPrefix = c.getNickname() + "!" + c.getUsername() + "@" + c.getHostname();
    std::string comment;
    if (msg.get_args().size() > 2)
        comment = msg.get_args()[2];
    else
        comment = c.getNickname();
    
    //on récupère la liste des clients qu'on stocke dans un vecteur
    //ca reste des std::string pour l'instant. Le check si ce sont bien des Clients est 
    //réalisé plus bas
    std::vector<std::string> clientsRaw = ft_split(',', msg.get_args()[1]);

    // Un message KICK distinct par utilisateur, diffusé à tout le channel.
    // Une fois le message envoyé, on supprime le client qui doit être Kick
    for (size_t i = 0; i < clientsRaw.size(); i++) //boucle sur le nombre de users à Kick
    {
        Client *target = findClientByNickname(clientsRaw[i]);
        if (target == NULL || !chan->isMember(*target))
        {
            send_reply_error(c, ERR_USERNOTINCHANNEL, clientsRaw[i], chan->getName(), "They aren't on that channel");
            continue; //on continue à traiter les autres users à KICK
        }
        std::string line = ":" + kickerPrefix + " KICK " + chan->getName() + " " + clientsRaw[i] + " :" + comment;
        broadcastToChannel(*chan, line); // envoyé à tous, y compris la cible, AVANT le kick

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
    std::string msg_to_send = ":" + c.getNickname() + "!" + c.getUsername() + "@localhost QUIT :" + reason;
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
    std::vector<std::string> args = msg.get_args();
    if (args.empty() || args[0].empty())
        return;
    Channel *chan = findChannelByName(args[0]);
    if (!chan)
    {
        Channel new_channel;
        new_channel.setName(args[0]);
        channels.insert(std::make_pair(args[0], new_channel));
        chan = findChannelByName(args[0]);
        chan->addMember(c);
        chan->addOperator(c);
    }
    else
    {
        if (chan->isMember(c))
        {
            send_reply_error(c, ERR_USERONCHANNEL, "you already joined the server");
            return;
        }
        chan->addMember(c);
        c.addChannel(*chan);
    }
}