/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_utilities.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 11:15:45 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/06/26 15:39:57 by bkaras-g         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <string>
#include <vector>

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
    if (channels_token == "") // si pas de channel trouvé, on renvoie un vecteur vide
        return (channels);

    // On récupère les noms des channels en retirant les préfixes '#' et '&'
    return (ft_split(',', channels_token));
}

std::vector<std::string> ft_split(char separator, const std::string &str)
{
    size_t sep_idx;
    size_t pos;
    std::vector<std::string> result;

    pos = 0;
    sep_idx = str.find(separator, pos);
    while (sep_idx != std::string::npos)
    {
        result.push_back(str.substr(pos, sep_idx - pos));
        pos = sep_idx + 1; // skip the separator
        sep_idx = str.find(separator, pos);
    }
    result.push_back(str.substr(pos));
    return (result);
}

// Vérifie que chaque channel de channelsToCheck existe sur le serveur.
// Le préfixe fait partie du nom : '#music' et '&music' sont deux channels distincts.
// @param channelsToCheck  Liste des noms de channels (avec préfixe) à vérifier.
// @return                 IRC_OK si tous les channels existent, ERR_NOSUCHCHANNEL sinon.
int Server::checkChannels(const std::vector<std::string> &channelsToCheck) const
{
    for (size_t i = 0; i < channelsToCheck.size(); i++)
    {
        if (this->channels.find(channelsToCheck[i]) == this->channels.end())
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
    return (findClientByNickname(nickname) != NULL);
}

/*
identifie un Client à partir de son nickname
@param nickname le nickname à identifier
@return Client* si trouvé; NULL si pas trouvé
Attention ! Le pointeur vers Client qui est return est valide tant que le vecteur
des clients n'est pas modifié avec un pushback() par exemple. A utiliser tout de suite.
*/
Client *Server::findClientByNickname(const std::string &nickname)
{

    for (size_t j = 0; j < this->vec_clients.size(); j++)
    {
        if (nickname == this->vec_clients[j].getNickname())
            return (&this->vec_clients[j]);
    }
    return (NULL);
}

/*
identifie un Channel à partir de son nom (préfixe inclus)
@param name le nom du channel à identifier (ex: "#music")
@return Channel* si trouvé; NULL si pas trouvé
Attention ! Le pointeur vers Channel qui est return est valide tant que le vecteur
des channels n'est pas modifié avec un pushback() par exemple. A utiliser tout de suite.
*/
Channel *Server::findChannelByName(const std::string &name)
{
    std::map<std::string, Channel>::iterator it = this->channels.find(name);
    if (it != this->channels.end())
        return (&it->second);
    return (NULL);
}

/*
Construit le préfixe d'origine d'un message IRC pour un Client: "nick!user@host".
Utilisé comme source des messages relayés (KICK, INVITE, etc.).
@param c le Client émetteur
@return le préfixe sous la forme "nickname!username@hostname"
*/
std::string Server::getPrefix(Client &c) const
{
    return (c.getNickname() + "!" + c.getUsername() + "@" + c.getHostname());
}

/*
Envoie une ligne IRC à tous les membres d'un channel.
Le \r\n final est ajouté par send_raw(), donc `line` ne doit pas le contenir.
@param chan     le channel dont les membres reçoivent la ligne
@param line     la ligne IRC complète à diffuser (ex: ":nick!user@host KICK #c bob :bye")
@param exclude  client à ne pas notifier (ex: l'émetteur pour PRIVMSG); NULL pour envoyer à tous
*/
void Server::broadcastToChannel(Channel &chan, const std::string &line, Client *exclude)
{
    const std::vector<Client *> &members = chan.getMembers();
    for (size_t i = 0; i < members.size(); i++)
    {
        if (exclude != NULL && members[i] == exclude)
            continue;
        send_raw(*members[i], line);
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

void Server::remove_client(int fd)
{
    vec_clients.erase(fd);
}