/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_utilities.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bkaras-g <bkaras-g@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/25 11:15:45 by bkaras-g          #+#    #+#             */
/*   Updated: 2026/07/14 12:52:10 by bkaras-g         ###   ########.fr       */
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
Attention ! Le pointeur vers Client qui est return est valide tant que la map
des clients n'est pas modifiée par suppression de l'entrée correspondante.
*/
Client *Server::findClientByNickname(const std::string &nickname)
{

    for (std::map<int, Client>::iterator it = this->vec_clients.begin(); it != this->vec_clients.end(); ++it)
    {
        if (nickname == it->second.getNickname())
            return (&it->second);
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
    return (c.getNickname() + "!" + c.getUsername() + "@localhost");
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
    if (dest.empty() || dest[0] != '#')
    {
        for (std::map<int, Client>::iterator it = vec_clients.begin(); it != vec_clients.end(); ++it)
        {
            if (it->second.getNickname() == dest)
                return it->second.getFdClient();
        }
    }
    return (-1);
}

void Server::remove_client(int fd)
{
    vec_clients.erase(fd);
}

void Server::remove_channel(std::string name)
{
    channels.erase(name);
}

// Affiché dès la connexion au socket, AVANT tout enregistrement.
// On utilise NOTICE (et non le MOTD) car le client n'a pas encore de nick.
void Server::sendConnectBanner(Client &c)
{
    static const char *header[] = {"╔════════════════════════════════════════════════════════════╗",
                                   "║                                                            ║",
                                   "║           ███████╗████████╗   ██╗██████╗  ██████╗          ║",
                                   "║           ██╔════╝╚══██╔══╝   ██║██╔══██╗██╔════╝          ║",
                                   "║           █████╗     ██║      ██║██████╔╝██║               ║",
                                   "║           ██╔══╝     ██║      ██║██╔══██╗██║               ║",
                                   "║           ██║        ██║      ██║██║  ██║╚██████╗          ║",
                                   "║           ╚═╝        ╚═╝      ╚═╝╚═╝  ╚═╝ ╚═════╝          ║",
                                   "║                                                            ║",
                                   "║              Internet Relay Chat — 42 Project              ║",
                                   "║                                                            ║",
                                   "╟────────────────────────────────────────────────────────────╢",
                                   "║                                                            ║",
                                   "║  This server lets IRC clients connect over TCP/IP to chat  ║",
                                   "║ in real time. Authenticate with the password, pick a nick, ║",
                                   "║ join channels, and talk. Operators can moderate channels.  ║",
                                   "║                                                            ║",
                                   "╟────────────────────────────────────────────────────────────╢",
                                   "║                                                            ║",
                                   "║                   ─── Basic commands ───                   ║",
                                   "║                                                            ║",
                                   "║   PASS <password>         authenticate to the server       ║",
                                   "║   NICK <nickname>         choose / change your nickname    ║",
                                   "║   USER <user> 0 * :name   register your username           ║",
                                   "║   JOIN #channel [key]     join (or create) a channel       ║",
                                   "║   PRIVMSG <target> :msg   message a user or a channel      ║",
                                   "║   TOPIC #channel [:txt]   view or set the channel topic    ║",
                                   "║   KICK #channel <nick>    eject a user      (operator)     ║",
                                   "║   INVITE <nick> #channel  invite a user     (operator)     ║",
                                   "║   MODE #channel +itkol    set channel modes (operator)     ║",
                                   "║   PART #channel           leave a channel                  ║",
                                   "║   QUIT :message           disconnect from the server       ║",
                                   "║                                                            ║",
                                   "╚════════════════════════════════════════════════════════════╝"};
    const size_t n = sizeof(header) / sizeof(header[0]);

    for (size_t i = 0; i < n; ++i)
    {
        std::string color = IC_CYAN; // cadre par défaut
        if (i >= 2 && i <= 7)        // le logo FT IRC
            color = std::string(I_BOLD) + IC_LCYAN;
        else if (i == 9) // le sous-titre
            color = IC_YELLOW;
        else if (i == 19) // "Basic commands"
            color = std::string(I_BOLD) + IC_YELLOW;

        // send_raw ajoute déjà le \r\n. Les codes couleur sont "largeur zéro"
        // dans irssi → l'alignement du cadre reste intact.
        send_raw(c, ":" + _server_name + " NOTICE * :" + color + header[i] + I_RESET);
    }
}