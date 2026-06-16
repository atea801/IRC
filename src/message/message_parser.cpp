#include "message.hpp"
#include "server.hpp"

/**
 * @brief une fonction qui vient remplir les attribut priver de la structure
 * cmd_args (command et argument) en ce referent a ce qu entre l utilisateur
 * dans sa boite de commande
 * 
 * @param data (buffer remplie par recv)
 * @return int 
 */
int
message::message_objects (std::string data)
{
  size_t pos = data.find ("\r\n");
  bool is_space = (data.find (' ') != std::string::npos);
  if (pos == std::string::npos)
    return (-1);
  int flag_com = 0;
  int j = 0;
  if (is_space == true)
    {
      for (int i = 0; i < pos; i++)
        {
          while (data[i] == ' ')
            i++;
          if (data[i] == ' ' && flag_com == 0)
            {
              flag_com = 1;
              this->command = data.substr (0, i);
              j = i + 1;
            }
          else if (data[i] == ' ' && flag_com == 1)
            {
              std::string arg = data.substr (j, i - j);
              this->args.push_back (arg);
              j = i + 1;
            }
          else if (data[i] == ':')
            {
              std::string arg = data.substr (i + 1, pos - (i + 1));
              this->args.push_back (arg);
              return (0);
            }
          else if (i == pos - 1)
            {
              std::string arg = data.substr (j, i - j + 1);
              this->args.push_back (arg);
              return (0);
            }
        }
    }
  else
    this->command = data.substr (0, pos);
  return (0);
}
