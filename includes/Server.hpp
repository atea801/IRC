#pragma once

#define I_RESET "\x0F"
#define I_BOLD "\x02"
#define IC_WHITE                                                                                                       \
    "\x03"                                                                                                             \
    "00"
#define IC_BLACK                                                                                                       \
    "\x03"                                                                                                             \
    "01"
#define IC_BLUE                                                                                                        \
    "\x03"                                                                                                             \
    "02"
#define IC_GREEN                                                                                                       \
    "\x03"                                                                                                             \
    "03"
#define IC_RED                                                                                                         \
    "\x03"                                                                                                             \
    "04"
#define IC_BROWN                                                                                                       \
    "\x03"                                                                                                             \
    "05"
#define IC_PURPLE                                                                                                      \
    "\x03"                                                                                                             \
    "06"
#define IC_ORANGE                                                                                                      \
    "\x03"                                                                                                             \
    "07"
#define IC_YELLOW                                                                                                      \
    "\x03"                                                                                                             \
    "08"
#define IC_LGREEN                                                                                                      \
    "\x03"                                                                                                             \
    "09"
#define IC_CYAN                                                                                                        \
    "\x03"                                                                                                             \
    "10"
#define IC_LCYAN                                                                                                       \
    "\x03"                                                                                                             \
    "11"
#define IC_LBLUE                                                                                                       \
    "\x03"                                                                                                             \
    "12"
#define IC_PINK                                                                                                        \
    "\x03"                                                                                                             \
    "13"
#define IC_GREY                                                                                                        \
    "\x03"                                                                                                             \
    "14"
#define IC_LGREY                                                                                                       \
    "\x03"                                                                                                             \
    "15"

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
#include <ctime>
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
#include <signal.h>

extern volatile sig_atomic_t g_stop;

class Server
{
  private:
    std::string _server_name;
    std::vector<pollfd> fds;
    std::map<int, Client> vec_clients;
    std::map<std::string, Channel> channels;
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
	void flush_client(int fd);

    /*--Gestion des commandes IRC--*/
    void exec_flow(Message &msg, Client &c);
    void handle_nick(Message &msg, Client &c);
    void handle_user(Message &msg, Client &c);
    void handle_pass(Message &msg, Client &c);
    void handle_privmsg(Message &msg, Client &c);
    void handle_cap(Message &msg, Client &c);
    void handle_ping(Message &msg, Client &c);
    void handle_quit(Message &msg, Client &c);
    void handle_join(Message &msg, Client &c);
    void handle_kick(Message &msg, Client &c);
    void handle_topic(Message &msg, Client &c);
    void handle_mode(Message &msg, Client &c);
    void handle_invite(Message &msg, Client &c);
    void handle_part(Message &msg, Client &c);

    /*--Fonctions utilitaires*/
    void remove_client(int fd);
    int find_dest(std::string dest);
    Client *find_client(int fd);
    Client *findClientByNickname(const std::string &nickname);
    int find_channel_index(std::string dest);
    Channel *findChannelByName(const std::string &name);
    std::vector<std::string> findChannelsInMsg(Message &msg);
    void broadcastToChannel(Channel &chan, const std::string &line, Client *exclude = NULL);
    std::string getPrefix(Client &c) const;
    void debug_client(Message &msg, Client &c);
    void sendConnectBanner(Client &client);
    void identify_and_exec_mode(Channel &chan, char sign, char mode_letter, const std::string &param);

    /*--Gestion des erreurs (Numeric replies)--*/
    int checkChannels(const std::vector<std::string> &channelsToCheck) const;
    bool checkSingleClientOnServer(std::string nickname);
    int checkMultipleClientsOnServer(const std::vector<std::string> &clientsToCheck);
    void send_reply_error(Client &c, IrcError error, const std::string &message);
    void send_reply_error(Client &c, IrcError error, const std::string &p1, const std::string &message);
    void send_reply_error(Client &c, IrcError error, const std::string &p1, const std::string &p2,
                          const std::string &message);
    void send_reply_channelmodeis(Client &c, Channel &chan);
    void send_mode_message(Client &c, Channel &chan, char sign, char mode_letter, const std::string &param);
};

// Fonction utilitaire libre (non-membre) : découpe `str` selon `separator`.
std::vector<std::string> ft_split(char separator, const std::string &str);

//Gestion des signaux pour arreter le serveur proprement
void ft_signal_handler(int signum);
void setup_signal_handlers();