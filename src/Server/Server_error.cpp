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
// void Server::send_raw(Client &c, const std::string &line){
// 	std::string mess = line + "\r\n";
// 	send(c.getFdClient(), mess.c_str(), mess.size(), MSG_EOR);
// }

/**
 * @brief Construire la partie toujours identique de chaque reponse
 * 
 * @param c 
 * @param error 
 * @return std::string 
 */
// std::string Server::reply_head(Client &c, IrcError error) const {
// 	int code =  static_cast<int>(error);

// 	std::string nickname = c.getNickname().empty() ? "*" : c.getNickname();
// 	std::ostringstream oss;

// 	oss << ":" << this->
// }