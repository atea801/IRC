#include "Message.hpp"


void Message::fill_cmd_and_args(size_t pos, std::string data)
{
    int j = 0;
    int flag_com = 0;
    for (size_t i = 0; i < pos; i++)
    {
        while (data[i] == ' ')
            i++;
        if (data[i] == ' ' && flag_com == 0)
        {
            flag_com = 1;
            this->command = data.substr(0, i);
            j = i + 1;
        }
        else if (data[i] == ' ' && flag_com == 1)
        {
            std::string arg = data.substr(j, i - j);
            this->args.push_back(arg);
            j = i + 1;
        }
        else if (data[i] == ':')
        {
            std::string arg = data.substr(i + 1, pos - (i + 1));
            this->args.push_back(arg);
        }
        else if (i == pos - 1)
        {
            std::string arg = data.substr(j, i - j + 1);
            this->args.push_back(arg);
        }
    }
}

/**
 * @brief une fonction qui vient remplir les attribut priver de la structure
 * cmd_args (command et argument) en ce referent a ce qu entre l utilisateur
 * dans sa boite de commande
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