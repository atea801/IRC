/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_utilities.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 11:15:45 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/25 14:11:15 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <string>
#include "Server.hpp"

// Cherche dans les arguments du message le premier token débutant par '#' ou '&',
// puis découpe ce token par ',' pour retourner la liste des noms de channels (sans leur préfixe).
// @param msg  Le message IRC parsé dont on analyse les arguments.
// @return     Un vecteur de noms de channels sans préfixe, ou un vecteur vide si aucun channel trouvé.
std::vector<std::string> Server::findChannelsInMsg(Message &msg)
{
    std::vector<std::string> channels;

    // On trouve la string qui contient les noms des channels pour la stocker 
    // dans channels_token
    std::string channels_token = "";
    for (size_t i = 0; i < msg.get_args().size(); i++)
    {
        if (!msg.get_args()[i].empty() && (msg.get_args()[i][0] == '#' || msg.get_args()[i][0] == '&'))
        {
            channels_token = msg.get_args()[i];
            break;
        }    
    }
    if (channels_token == "")
        return (channels);
        
    //On récupère les noms des channels en retirant les préfixes '#' et '&'
    size_t comma_idx;
    size_t pos;
    pos = 1;
    comma_idx = channels_token.find(',', pos);
    while (comma_idx != std::string::npos)
    {
        channels.push_back(channels_token.substr(pos, comma_idx - pos));
        pos = comma_idx + 2; //skip the ',' and the '#' or '&'
        comma_idx = channels_token.find(',', pos);
    }
    channels.push_back(channels_token.substr(pos));
    return (channels);
}

int Server::checkChannels(const std::vector<std::string> &channelsToCheck) const
{
    bool found;

    for (size_t i = 0; i < channelsToCheck.size(); i++)
    {
        found = false;
        for (size_t j = 0; j < this->channels.size(); j++)
        {
            if (channelsToCheck[i] == this->channels[j].getName())
            {
                found = true;
                break;
            }    
        }
        if (found == false)
            return (ERR_NOSUCHCHANNEL);
    }
    return (IRC_OK);
}