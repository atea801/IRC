#include "Message.hpp"

IrcError Message::parsing_nick(){
	//1 arg est vide
	if (this->args.empty())
		return ERR_NICK_EMPTY;
	//2 check qu'il y a bien qu'un seul str dans le vecteur
	if (this->args.size() > 1 || args[0].size() > 9)
		return ERR_NICK_INVALID;
	//3 check premier caractere
	std::string special = "-_[]{}\\|";
	if (!isalpha(args[0][0]) && special.find(this->args[0][0]) == std::string::npos)
			return ERR_NICK_INVALID;
	//3 check caracteres
	for (size_t i = 1; i < this->args[0].size(); i++){
		//si le caractere n'est pas une lettre/chiffre et qu'il n'est pas dans les car speciaux
		if (!isalnum(args[0][i]) && special.find(this->args[0][i]) == std::string::npos)
			return ERR_NICK_INVALID;
	}
	return IRC_OK;
}