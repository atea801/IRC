#pragma once

#include "Channel.hpp"
#include "Client.hpp"
#include "Message.hpp"
#include "PtrVec.hpp"
#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class Server
{
  private:
    std::string _server_name;
    std::vector<pollfd> fds;
    std::map<int, Client> vec_clients;
    std::vector<Channel> channels;
    std::string port;
    std::string password;
    int server_fd;

    void send_raw(Client &c, const std::string &line);
    std::string reply_head(Client &c, IrcError error) const;

  public:
    /*--Constructeurs-Destructeur--*/
    Server(std::string port, std::string password);
    ~Server();
    Server(const Server &copy);
    Server &operator=(const Server &other);

    /*--Getters--*/
    const std::string &getPort() const;
    const std::string &getPassword() const;

    /*--Setters--*/
    void setPort(const std::string &port);
    void setPassword(const std::string &password);

    /*--création et gestion de la connection Client-Serveur--*/
    int check_port(char *port);
    int init_server(char **av);
    int run();
    int accept_new_client();
    int create_socket();
    int client_actions(size_t i);

    /*--Gestion des commandes IRC--*/
    void exec_flow(Message &msg, Client &c);
    void handle_nick(Message &msg, Client &c);
    void handle_user(Message &msg, Client &c);
    void handle_pass(Message &msg, Client &c);
    void handle_privmsg(Message &msg, Client &c);
    void handle_cap(Client &c);
    void handle_ping(Message &msg, Client &c);
    void handle_quit(Message &msg, Client &c);
    void handle_join(Message &msg, Client &c);
    void handle_Kick(Message &msg, Client &c); // K pour distinguer du n de handle_nick()
    void handle_mode(Message &msg, Client &c);

    /*--Fonctions utilitaires*/
    int find_channel(std::string dest);
    void remove_client(int fd);
    int find_dest(std::string dest);
    Client *find_client(std::vector<pollfd> fds, size_t i);
    Client* findClientByNickname(const std::string &nickname);
    int find_channel_index(std::string dest);
    Channel *findChannelByName(const std::string &name);
    std::vector<std::string> findChannelsInMsg(Message &msg);
    void broadcastToChannel(Channel &chan, const std::string &line, Client *exclude = NULL);

    /*--Gestion des erreurs (Numeric replies)--*/
    int checkChannels(const std::vector<std::string> &channelsToCheck) const;
    bool checkSingleClientOnServer(std::string nickname);
    int checkMultipleClientsOnServer(const std::vector<std::string> &clientsToCheck);
    void send_reply_error(Client &c, IrcError error, const std::string &message);
    void send_reply_error(Client &c, IrcError error, const std::string &p1, const std::string &message);
    void send_reply_error(Client &c, IrcError error, const std::string &p1, const std::string &p2,
                          const std::string &message);
};

// Fonction utilitaire libre (non-membre) : découpe `str` selon `separator`.
std::vector<std::string> ft_split(char separator, const std::string &str);
