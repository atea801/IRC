#include "Message.hpp"

IrcError Message::parsing_nick()
{
    // 1 Aucun nick fourni (vecteur vide ou chaîne vide après ':')
    if (this->args.empty() || this->args[0].empty())
        return ERR_NONICKNAMEGIVEN;
    // Les autres args sont ignore
    const std::string &nick = this->args[0];
    // 2 check de la longueur
    if (nick.size() > 10)
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
    // if (!this->trailing_arg)
    // 	return ERR_NEEDMOREPARAMS;
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

IrcError Message::parsing_privmsg()
{
    if (this->args.empty() || this->args[0].empty())
    {
        return ERR_NORECIPIENT;
    }
    if (this->args.size() < 2 || args[1].empty())
        return ERR_NOTEXTTOSEND;
    return IRC_OK;
}

IrcError Message::parsing_kick()
{
    // KICK doit avoir au moins <channel> et <user>
    if (this->args.size() < 2)
        return ERR_NEEDMOREPARAMS;
    if (this->args[0].empty())
        return ERR_NEEDMOREPARAMS;
    if (this->args[1].empty())
        return ERR_NEEDMOREPARAMS;

    // On vérifie qu'après split sur les virgules, il reste au moins
    // une cible non vide (rejette "KICK #chan ," ou "KICK #chan ,,")
    std::vector<std::string> targets = ft_split(',', this->args[1]);
    bool hasValidTarget = false;
    for (size_t i = 0; i < targets.size(); i++)
    {
        if (!targets[i].empty())
        {
            hasValidTarget = true;
            break;
        }
    }
    if (!hasValidTarget)
        return ERR_NEEDMOREPARAMS;

    return IRC_OK;
}

IrcError Message::parsing_invite()
{
	if (this->args.size() < 2)
        return ERR_NEEDMOREPARAMS;
    return IRC_OK;
}

IrcError Message::parsing_mode()
{
    // 1. Il faut au moins la cible (le channel)
    if (this->args.empty() || this->args[0].empty())
        return ERR_NEEDMOREPARAMS;

    // 2. "MODE #chan" seul = requête des modes actifs -> valide,
    //    handle_mode s'en occupe via send_reply_channelmodeis
    if (this->args.size() == 1)
        return IRC_OK;

    const std::string &modestring = this->args[1];
    if (modestring.empty())
        return ERR_NEEDMOREPARAMS;

    // 3. On parcourt le modestring : on valide chaque lettre
    //    et on compte les <mode arguments> requis.
    //    Meme logique que modeNeedsParam : k,o -> toujours ; l -> seulement '+'.
    char sign = '+'; // signe courant, '+' par defaut si aucun signe explicite
    size_t needed = 0;
    for (size_t i = 0; i < modestring.size(); i++)
    {
        char ch = modestring[i];
        if (ch == '+' || ch == '-')
        {
            sign = ch;
            continue;
        }
        if (ch != 'i' && ch != 't' && ch != 'k' && ch != 'o' && ch != 'l')
            return ERR_UNKNOWNMODE;
        if (ch == 'k' || ch == 'o')
            needed++;
        else if (ch == 'l' && sign == '+')
            needed++;
    }

    // 4. Les <mode arguments> sont args[2..]. Il en faut au moins `needed`.
    size_t available = (this->args.size() > 2) ? this->args.size() - 2 : 0;
    if (available < needed)
        return ERR_NEEDMOREPARAMS;

    return IRC_OK;
}