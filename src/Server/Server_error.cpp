#include "Server.hpp"

/**
 * @brief
 *
 * :server 451 nick :You have not registered          → 0 param intermédiaire
 * :server 461 nick JOIN :Not enough parameters        → 1 param  (la commande)
 * :server 442 nick #chan :You're not on that channel  → 1 param  (le channel)
 * :server 441 nick bob #chan :They aren't on that...   → 2 params (nick + channel)
 */

/**
 * @brief Prend une ligne deja construite et lenvoyer sur le socker
 * du client - ne construit rien - recoit du texte deja pret et envoie sur le socket
 *
 * - permet de mettre le \r\n a la in obligatoire de chaque fin de message
 *
 * - envoie propre avec poll()/POLLOUT on a qu'une seul fonction a modifier
 * au lieu de passer par send() directement send_raw empilera les objets dans c buffer
 *
 * - on ajoute le \r\n ici pour eviter de le rajouter avant a la main = risque d'oubli
 * @param c
 * @param line ligne IRC complete mais sans le \r\n final
 */
void Server::send_raw(Client &c, const std::string &line)
{
    std::string mess = line + "\r\n";
    send(c.getFdClient(), mess.c_str(), mess.size(), 0);
}

/**
 * @brief Construire la partie toujours identique de chaque reponse
 *
 *  <server_name> <code error> <nickname> <le reste qui va changer>
 * @param c
 * @param error
 * @return std::string
 */
std::string Server::reply_head(Client &c, IrcError error) const
{
    int code = static_cast<int>(error);

    std::string nickname = c.getNickname().empty() ? "*" : c.getNickname();
    std::ostringstream oss;

    oss << ":" << this->_server_name << " " << code << " " << nickname;
    return oss.str();
}

/**
 * @brief Message de base avec 0 parametre <reply head> : <le message>
 *
 * @code
 * :<server_name> <code> <nickname> :<message>\r\n
 * @endcode
 * @param c Client destinataire de la réponse
 * @param error Code numérique de l'erreur
 * @param message Texte explicatif placé après le ':'
 */
void Server::send_reply_error(Client &c, IrcError error, const std::string &message)
{
    send_raw(c, reply_head(c, error) + " :" + message);
}

/**
 * @brief Message de base avec 1 parametre <reply head> <P1>: <le message>
 *
 * @code
 * :<server_name> <code> <nickname> <P1> :<message>\r\n
 * @endcode
 * @param c Client destinataire de la réponse
 * @param error Code numérique de l'erreur
 * @param p1 Premier paramètre (ex. : la commande, le channel, un nick)
 * @param message Texte explicatif placé après le ':'
 */
void Server::send_reply_error(Client &c, IrcError error, const std::string &p1, const std::string &message)
{
    send_raw(c, reply_head(c, error) + " " + p1 + " :" + message);
}

/**
 * @brief Message de base avec 1 parametre <reply head> <P1>: <le message>
 *
 * @code
 * :<server_name> <code> <nickname> <P1> <P2>:<message>\r\n
 * @endcode
 * @param c Client destinataire de la réponse
 * @param error Code numérique de l'erreur
 * @param p1 Premier paramètre (ex. : la commande, le channel, un nick)
 * @param p2 Second paramètre (dans l'ordre du message IRC)
 * @param message Texte explicatif placé après le ':'
 */
void Server::send_reply_error(Client &c, IrcError error, const std::string &p1, const std::string &p2,
                              const std::string &message)
{
    send_raw(c, reply_head(c, error) + " " + p1 + " " + p2 + " :" + message);
}
