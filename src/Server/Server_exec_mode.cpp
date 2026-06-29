/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_exec_mode.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/27 17:52:49 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/29 13:50:39 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void identify_and_exec_mode(Channel &chan, Client &c, char sign, char mode_letter, const std::string &param);
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

    //A partir de ce point, on considère que <modestring> est au bon format et que
    //<mode arguments> contient le bon nombre d'arguments pour chaque mode demandé
    //dans <modestring> (Voir sur Figma)
    //Attention au mode -k qui nécessite un <mode argument> même s'il n'est pas utile
    
    //Check des numeric replies
    Channel *chan = findChannelByName(msg.get_args()[0]);
    if (!chan)
    {
        // ERR_NOSUCHCHANNEL (403)
        // send_reply_error "<client> <channel> :No such channel"
        return;
    }
    if (msg.get_args().size() == 1) // MODE #general --> demande les modes activés
    {
        // RPL_CHANNELMODEIS (324)
        //"<client> <channel> <modestring> <mode arguments>..."
        // exemple avec tous les modes possibles: ":irc.42.fr 324 dan #music +itkl secret 42"
        // secret est le password (+k)
        // 42 est le user limit (+l)
        // i et t ne donnent pas de mode arguments
        // Attention: le mode +o est exclu de ce num reply
    }
    if (!chan->isOperator(c))
    {
        // ERR_CHANOPRIVSNEEDED (482)
        // send_reply_error "<client> <channel> :You're not channel operator"
        return;
    }

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
        if (*it == 'o') //check num reply : si le client à ajouter en ChanOps est dans le channel
        {
            if (findClientByNickname(mode_args[args_idx]) == NULL)
            {
                // ERR_USERNOTINCHANNEL (441) "<client> <nick> <channel> :They aren't on that channel"
                args_idx++;
                // PAS DE return car on continue l'exec des autres modes demandés
                // on peut avoir par ex "MODE +ok-i Bob secret" et meme si Bob ne fait pas partie du channel
                // on doit continuer à traiter les autres modes
            }
        }
        std::string param;
        if (modeNeedsParam(sign, *it)) //check si le mode identifié nécessite un <mode argument>
            param = mode_args[args_idx++]; // pas de check si mode_args contient bien des args car fait au parsing
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
            chan.addOperator(c);
        else
            chan.removeOperator(c);
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