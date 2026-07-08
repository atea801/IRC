#pragma once

#include "PtrVec.hpp"
#include "utils.hpp"
#include <iostream>

class Channel;

enum e_CLientState
{
    HANDSHAKE,
    REGISTERED,
    QUIT,
};

class Client
{
  private:
    int _fdclient; // no setter set at construction only
    std::string _nickname;
    std::string _realname;
    std::string _username;
    std::string _hostname; // adresse ip du client
    std::string _buffer;   // permet de stocker les morceaux envoyer par recv()
	std::string _outBuffer; // stock ce que le serveur veut envoyer en attendant que le noyau l'ecrive
    PtrVec<Channel> _client_channels;
    e_CLientState _status;
    bool _bool_pass;
    bool _bool_nick;
    bool _bool_user;


  public:
    /*--Constructeurs-Destructeur--*/
    Client();
    Client(int fd_input);
    Client(std::string hostname_input);
    Client(const Client &copy);
    Client &operator=(const Client &other);
    bool operator==(const Client &other);
    ~Client();

    /*--Getters--*/
    const int &getFdClient() const;
    const std::string &getNickname() const;
    const std::string &getRealname() const;
    const std::string &getUsername() const;
    const std::string &getHostname() const;
    const std::string &getBuffer() const;
    const e_CLientState &getStatus() const;
    const bool &getBoolPass() const;
    const bool &getBoolNick() const;
    const bool &getBoolUser() const;
    const PtrVec<Channel> &get_client_channel() const;

    /*--Setters--*/
    void setNickname(std::string const nickname_input);
    void setRealname(std::string const realname_input);
    void setUsername(std::string const username_input);
    void setHostname(std::string const hostname_input);
    void setBuffer(std::string const buffer_input);
    void setStatus(e_CLientState const status_input);
    void setBoolPass(bool const pass);
    void setBoolNick(bool const nick);
    void setBoolUser(bool const user);
    void addChannel(Channel &new_channel);
    void removeChannel(Channel &old_channel);

    /*-outBuffer--*/
	void appendOut(const std::string &str);
	const std::string &getOut() const;
	void consumeOut(size_t n);
};