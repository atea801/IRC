/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_utilities.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 11:15:45 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/25 16:59:19 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <vector>
#include <string>
#include "Server.hpp"

// Cherche dans les arguments du message le premier token débutant par '#' ou '&',
// puis découpe ce token par ',' pour retourner la liste des noms de channels (avec leur préfixe).
// @param msg  Le message IRC parsé dont on analyse les arguments.
// @return     Un vecteur de noms de channels avec leur préfixe, ou un vecteur vide si aucun channel trouvé.
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
    pos = 0;
    comma_idx = channels_token.find(',', pos);
    while (comma_idx != std::string::npos)
    {
        channels.push_back(channels_token.substr(pos, comma_idx - pos));
        pos = comma_idx + 1; //skip the ',' and the '#' or '&'
        comma_idx = channels_token.find(',', pos);
    }
    channels.push_back(channels_token.substr(pos));
    return (channels);
}

// Vérifie que chaque channel de channelsToCheck existe sur le serveur.
// Le préfixe fait partie du nom : '#music' et '&music' sont deux channels distincts.
// @param channelsToCheck  Liste des noms de channels (avec préfixe) à vérifier.
// @return                 IRC_OK si tous les channels existent, ERR_NOSUCHCHANNEL sinon.
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

// Vérifie que chaque client de clientsToCheck existe sur le serveur (par son nickname).
// @param clientsToCheck  Liste des nicknames à vérifier.
// @return                IRC_OK si tous les clients existent, ERR_NOSUCHNICK sinon.
int Server::checkMultipleClientsOnServer(const std::vector<std::string> &clientsToCheck)
{
    bool found;

    for (size_t i = 0; i < clientsToCheck.size(); i++)
    {
        found = this->checkSingleClientOnServer(clientsToCheck[i]);
        if (found == false)
            return (ERR_NOSUCHNICK);
    }
    return (IRC_OK);
}

bool Server::checkSingleClientOnServer(std::string nickname)
{
    for (size_t j = 0; j < this->vec_clients.size(); j++)
    {
        if (nickname == this->vec_clients[j].getNickname())
            return (true);
    }
    return (false);
}