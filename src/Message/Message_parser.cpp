#include "Message.hpp"

IrcError Message::parsing_nick()
{
    // 1 Aucun nick fourni (vecteur vide ou chaîne vide après ':')
    if (this->args.empty() || this->args[0].empty())
        return ERR_NONICKNAMEGIVEN;
    // Les autres args sont ignore
    const std::string &nick = this->args[0];
    // 2 check de la longueur
    if (nick.size() > 9)
        return ERR_ERRONEUSNICKNAME;
    // 3 check premier caractere
    std::string special = "-_[]{}\\|";
    if (!isalpha(nick[0]) && special.find(nick[0]) == std::string::npos)
        return ERR_ERRONEUSNICKNAME;
    // 3 check caracteres
    for (size_t i = 1; i < nick.size(); i++)
    {
        // si le caractere n'est pas une lettre/chiffre et qu'il n'est pas dans les car speciaux
        if (!isalnum(nick[i]) && special.find(nick[i]) == std::string::npos)
            return ERR_ERRONEUSNICKNAME;
    }
    return IRC_OK;
}

IrcError Message::parsing_user()
{
    if (this->args.empty() || this->args.size() != 4)
        return ERR_NEEDMOREPARAMS;
    std::string special = " @!";
    for (size_t i = 0; i < this->args[0].size(); i++)
    {
        if (special.find(args[0][i]) != std::string::npos)
            return ERR_INVALID;
    }
    // if (this->args[1].size() != 1 || this->args[2].size() != 1)
    // 	return ERR_INVALID;
    // if (args[1][0] != '0' || args[2][0] != '*')
    // 	return ERR_INVALID;
    return IRC_OK;
}

IrcError Message::parsing_pass()
{
    if (this->args.empty() || args[0].empty())
        return ERR_NEEDMOREPARAMS;
    return IRC_OK;
}

/**
 * @brief Il n'y a pas de cas ou le parisng est invalid
 * QUIT
 * QUIT :bye
 * QUIT :Gone to have lunch
 *
 * @return IrcError
 */
IrcError Message::parsing_quit()
{
    return IRC_OK;
}