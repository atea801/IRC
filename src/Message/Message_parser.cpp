#include "Message.hpp"

IrcError Message::parsing_nick()
{
    // 1 arg est vide
    if (this->args.empty())
        return ERR_NONICKNAMEGIVEN;
    // 2 check qu'il y a bien qu'un seul str dans le vecteur
    if (this->args.size() > 1 || args[0].size() > 9)
        return ERR_INVALID;
    // 3 check premier caractere
    std::string special = "-_[]{}\\|";
    if (!isalpha(args[0][0]) && special.find(this->args[0][0]) == std::string::npos)
        return ERR_ERRONEUSNICKNAME;
    // 3 check caracteres
    for (size_t i = 1; i < this->args[0].size(); i++)
    {
        // si le caractere n'est pas une lettre/chiffre et qu'il n'est pas dans les car speciaux
        if (!isalnum(args[0][i]) && special.find(this->args[0][i]) == std::string::npos)
        return ERR_ERRONEUSNICKNAME;
    }
    return IRC_OK;
}


IrcError Message::parsing_user(){
	if (this->args.empty())
		return ERR_EMPTY;
	if (this->args.size() != 4)
		return ERR_NEEDMOREPARAMS;
	std::string special = " @!";
	for (size_t i = 0; i < this->args[0].size(); i++){
		if (special.find(args[0][i]) != std::string::npos)
			return ERR_INVALID;
	}
	// if (this->args[1].size() != 1 || this->args[2].size() != 1)
	// 	return ERR_INVALID;
	// if (args[1][0] != '0' || args[2][0] != '*')
	// 	return ERR_INVALID;
	return IRC_OK;
}

IrcError Message::parsing_pass(){
	if (this->args.empty())
		return ERR_EMPTY;
	if (this->args.size() != 1)
		return ERR_NEEDMOREPARAMS;
	if (args[0].empty())
		return ERR_NEEDMOREPARAMS;
	return IRC_OK;
}

IrcError Message::parsing_quit()
{
	if (this->args.empty())
        return ERR_EMPTY;
	if (this->args.size() != 1)
		return ERR_NEEDMOREPARAMS;
    if (this->args[0].empty())
		return ERR_NEEDMOREPARAMS;
	return IRC_OK;
}