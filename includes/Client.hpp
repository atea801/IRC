#pragma once

#include <iostream>

enum e_CLientState
{
    HANDSHAKE,
    PASS, // mot de passe
    NICK, // nickname
    USER,
    REGISTERED,
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
    e_CLientState _status;

  public:
    /*--Constructeurs-Destructeur--*/
    Client();
    Client(int fd_input);
    Client(std::string hostname_input);
    Client(const Client &copy);
    Client &operator=(const Client &other);
    ~Client();

    /*--Getters--*/
    const int &getFdClient() const;
    const std::string &getNickname() const;
    const std::string &getRealname() const;
    const std::string &getUsername() const;
    const std::string &getHostname() const;
    const std::string &getBuffer() const;
    const e_CLientState &getStatus() const;

    /*--Setters--*/
    void setNickname(std::string const nickname_input);
    void setRealname(std::string const realname_input);
    void setUsername(std::string const username_input);
    void setHostname(std::string const hostname_input);
    void setBuffer(std::string const buffer_input);
    void setStatus(e_CLientState const status_input);
};