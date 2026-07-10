/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_exec_mode.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/27 17:52:49 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/07/10 14:38:01 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

static bool modeNeedsParam(char sign, char letter);

/*
Traitement de l'exec de MODE. Format : `<target> [<modestring> [<mode arguments>...]]`
Exemple : `MODE #chan +itk-l+o secretkey alice`

Le besoin en <mode arguments> dépend des modes demandés. Voir sur Figma
les besoins en <mode arguments> et le diagramme d'exec flow suivi par cette fonction.

Après check des numeric replies, extractions des paramètres <modestring>
et <mode arguments> avec msg.get_args()
Puis on parcourt <modestring> et <mode arguments> pour traiter l'exec de chaque mode
@param &msg Le Message de MODE
@param &c Le Client émetteur du Message
*/
void Server::handle_mode(Message &msg, Client &c)
{
    IrcError error = msg.parsing_mode();
    if (error != IRC_OK)
    {
        if(error == ERR_NEEDMOREPARAMS)
            return (send_reply_error(c, error, msg.get_command(), "Not enough parameters"));
        else if(error == ERR_UNKNOWNMODE)
        	return(send_reply_error(c, error, msg.get_args()[1], "is unknown mode char to me"));
    }

    // A partir de ce point, on considère que <modestring> est au bon format et que
    //<mode arguments> contient le bon nombre d'arguments pour chaque mode demandé
    // dans <modestring> (Voir sur Figma)
    // Attention au mode -k qui nécessite un <mode argument> même s'il n'est pas utile

    // Check des numeric replies
    Channel *chan = findChannelByName(msg.get_args()[0]);
    if (!chan)
        return (send_reply_error(c, ERR_NOSUCHCHANNEL, msg.get_args()[0], "No such channel"));
    if (msg.get_args().size() == 1) // MODE #general --> demande les modes activés
       return(send_reply_channelmodeis(c, *chan));
    if (!chan->isOperator(c))
        return(send_reply_error(c, ERR_CHANOPRIVSNEEDED, chan->getName(), "You're not channel operator"));
    /*
    Extraction des paramètres <modestring> et <mode arguments> avec msg.get_args()
    */
    std::string modestring = msg.get_args()[1];
    std::vector<std::string> mode_args;
    const std::vector<std::string> &args = msg.get_args();

    for (size_t i = 2; i < args.size(); i++)
        mode_args.push_back(args[i]);

    /*
    On parcourt <modestring> et <mode arguments> pour traiter l'exec de chaque mode
    Identification du mode avec l'itérateur it sur modestring
    */
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
        if (*it == 'o') // check num reply : si le client à ajouter en ChanOps est dans le channel
        {
            Client *target = findClientByNickname(mode_args[args_idx]);
            if (target == NULL || !chan->isMember(*target))
            {
                // ERR_USERNOTINCHANNEL (441) "<client> <nick> <channel> :They aren't on that channel"
                send_reply_error(c, ERR_USERNOTINCHANNEL, mode_args[args_idx], chan->getName(),
                                 "They aren't on that channel");
                args_idx++;
                it++;
                continue; // PAS DE return car on continue l'exec des autres modes demandés
                // on peut avoir par ex "MODE +ok-i Bob secret" et meme si Bob ne fait pas partie du channel
                // on doit continuer à traiter les autres modes
            }
        }
        std::string param = "";
        if (modeNeedsParam(sign, *it))     // check si le mode identifié nécessite un <mode argument>
            param = mode_args[args_idx++]; // pas de check si mode_args contient bien des args car fait au parsing
        identify_and_exec_mode(*chan, sign, *it, param);
        send_mode_message(c, *chan, sign, *it, param);
        it++;
    }
}

void Server::identify_and_exec_mode(Channel &chan, char sign, char mode_letter, const std::string &param)
{
    bool set;
    if (sign == '+')
        set = true;
    else
        set = false;
    switch (mode_letter)
    {
    case 'i': {
        chan.setInviteOnly(set);
        break;
    }
    case 't': {
        chan.setTopicRestricted(set);
        break;
    }
    case 'k': {
        if (set == true)
            chan.setPassword(param);
        else
            chan.removePassword();
        break;
    }
    case 'o': {
        if (set == true)
            chan.addOperator(*findClientByNickname(param));
        else
            chan.removeOperator(*findClientByNickname(param));
        break;
    }
    case 'l': {
        if (set == true)
            chan.setUserLimit(std::atoi(param.c_str()));
        else
            chan.removeUserLimit();
        break;
    }
    }
}

/*
Construit et envoie le numeric reply RPL_CHANNELMODEIS (324) qui liste les modes
actuellement activés sur le channel, suivi de leurs <mode arguments> dans le même ordre.
Format: ":<server> 324 <client> <channel> <modestring> <mode arguments>..."
Ordre des modes: i, t, k, l (le mode +o n'apparaît PAS dans ce reply).
Seuls k (password) et l (user limit) ajoutent un argument.
Même logique d'envoi que send_reply_error() : reply_head() + send_raw().
*/
void Server::send_reply_channelmodeis(Client &c, Channel &chan)
{
    std::string modestring = "+";
    std::string arguments = "";

    if (chan.isInviteOnly())
        modestring += "i";
    if (chan.isTopicRestricted())
        modestring += "t";
    if (chan.hasPassword())
    {
        modestring += "k";
        arguments += " " + chan.getPassword();
    }
    if (chan.hasUserLimit())
    {
        modestring += "l";
        std::ostringstream oss;
        oss << chan.getUserLimit();
        arguments += " " + oss.str();
    }
    send_raw(c, reply_head(c, RPL_CHANNELMODEIS) + " " + chan.getName() + " " + modestring + arguments);
}

void Server::send_mode_message(Client &c, Channel &chan, char sign, char mode_letter, const std::string &param)
{
    std::string prefix = getPrefix(c);
    std::string message = ":" + prefix;
    if (param != "")
        message += " MODE " + chan.getName() + " " + sign + mode_letter + " " + param;
    else
        message += " MODE " + chan.getName() + " " + sign + mode_letter;
    broadcastToChannel(chan, message, NULL);
}

// Mode k, o: param on '+' AND '-'.
// Mode l: param only on '+'.
// Mode i, t: no param.
static bool modeNeedsParam(char sign, char letter)
{
    if (letter == 'k' || letter == 'o')
        return true;
    if (letter == 'l')
        return (sign == '+');
    return false;
}