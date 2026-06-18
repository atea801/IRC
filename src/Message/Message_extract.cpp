#include "Message.hpp"

/**
 * @brief remplie les attributs priver commande et args de la class message
 *
 * @param pos
 * @param data
 */

void Message::fill_cmd_and_args(size_t pos, std::string data)
{
    size_t i = 0;
    
    // 1. extraire la commande jusqu'au premier espace
    size_t space = data.find(' ');
    if (space == std::string::npos || space >= pos)
    {
        // pas d'espace → que la commande, pas d'args
        this->command = data.substr(0, pos);
        return ;
    }
    this->command = data.substr(0, space);
    i = space + 1;

    // 2. extraire les args
    while (i < pos)
    {
        // sauter les espaces
        while (i < pos && data[i] == ' ')
            i++;
        if (i >= pos)
            break ;
        // trailing
        if (data[i] == ':')
        {
            this->args.push_back(data.substr(i + 1, pos - i - 1));
            this->trailing_arg = true;
            break ;
        }
        // arg normal
        size_t next_space = data.find(' ', i);
        if (next_space == std::string::npos || next_space >= pos)
        {
            this->args.push_back(data.substr(i, pos - i));
            break ;
        }
        this->args.push_back(data.substr(i, next_space - i));
        i = next_space + 1;
    }
}


/**
 * @brief extraction d elements commande et args a partir
 * du buffer d un client precis et suppression des elements
 * apres recuperation. permet de garder le buffer client a jour.
 *
 * @param data (buffer remplie par recv)
 * @return int
 */
void Message::extract_and_clean(Client &c)
{
    // 1. extraire la premiere commande
    std::string data = c.getBuffer();
    size_t pos = data.find("\r\n");
    bool is_space = (data.find(' ') != std::string::npos);
    if (is_space == true)
        fill_cmd_and_args(pos, data);
    else
        this->command = data.substr(0, pos);
    // 2. nettoie le buf
    c.setBuffer(data.substr(pos + 2));
}
