#include "Message.hpp"

/**
 * @brief remplie les attributs priver commande et args de la class message
 *
 * @param pos
 * @param data
 */
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
            this->trailing_arg = true;
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

/**
 * @brief parsing de l attribut prive command de la class message
 *
 * @param cmd
 * @return int
 */
int Message::parser_cmd(std::string cmd)
{
    if (cmd == "NICK" || cmd == "QUIT" || cmd == "JOIN" || cmd == "PASS" || cmd == "USER" || cmd == "PART" ||
        cmd == "TOPIC" || cmd == "MODE" || cmd == "INVITE" || cmd == "KICK" || cmd == "PRIVMSG" || cmd == "PING" ||
        cmd == "PONG" || cmd == "CAP")
        return (0);
    return (-1);
}

int Message::handle_quit(std::vector<std::string> args)
{
    if (args.size() == 0)
        return (0);
    else if (args.size() == 1 && this->trailing_arg == true)
    {
        std::string arg = args[0];
        for (int i = 0; i < arg.size(); i++)
        {
            
        }
        return (1);
    }
    else
        return (-1);
}

