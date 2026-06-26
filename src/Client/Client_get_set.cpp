#include "Client.hpp"

const int &Client::getFdClient() const
{
    return this->_fdclient;
}

const std::string &Client::getNickname() const
{
    return this->_nickname;
}

const std::string &Client::getRealname() const
{
    return this->_realname;
}

const std::string &Client::getUsername() const
{
    return this->_username;
}

const std::string &Client::getHostname() const
{
    return this->_hostname;
}

const std::string &Client::getBuffer() const
{
    return this->_buffer;
}

const e_CLientState &Client::getStatus() const
{
    return this->_status;
}

const bool &Client::getBoolPass() const
{
    return this->_bool_pass;
}

const bool &Client::getBoolNick() const
{
    return this->_bool_nick;
}

const bool &Client::getBoolUser() const
{
    return this->_bool_user;
}

void Client::setNickname(std::string const nickname_input)
{
    this->_nickname = nickname_input;
}

void Client::setRealname(std::string const realname_input)
{
    this->_realname = realname_input;
}

void Client::setUsername(std::string const username_input)
{
    this->_username = username_input;
}

void Client::setHostname(std::string const hostname_input)
{
    this->_hostname = hostname_input;
}

void Client::setBuffer(std::string const buffer_input)
{
    this->_buffer = buffer_input;
}

void Client::setStatus(e_CLientState const status_input)
{
    this->_status = status_input;
}

void Client::setBoolPass(bool const pass)
{
    this->_bool_pass = pass;
}

void Client::setBoolUser(bool const user)
{
    this->_bool_user = user;
}

void Client::setBoolNick(bool const nick)
{
    this->_bool_nick = nick;
}

void Client::addChannel(Channel &new_channel) 
{
    this->_client_channels.add(new_channel);
}
