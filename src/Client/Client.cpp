#include "Client.hpp"

Client::Client()
    : _fdclient(-1), _nickname(""), _realname(""), _username(""), _hostname(""), _buffer(""), _status(HANDSHAKE),
	_bool_pass(false), _bool_nick(false), _bool_user(false)
{
}

Client::Client(int fd_input)
    : _fdclient(fd_input), _nickname(""), _realname(""), _username(""), _hostname(""), _buffer(""), _status(HANDSHAKE),
	_bool_pass(false), _bool_nick(false), _bool_user(false)
{
}

Client::Client(std::string hostname_input)
    : _fdclient(-1), _nickname(""), _realname(""), _username(""), _hostname(hostname_input), _buffer(""),
      _status(HANDSHAKE), _bool_pass(false), _bool_nick(false), _bool_user(false)
{
}

Client::Client(const Client &copy)
{
    *this = copy;
}

Client &Client::operator=(const Client &copy)
{
    if (this != &copy)
    {
        this->_fdclient = copy.getFdClient();
        this->_nickname = copy.getNickname();
        this->_realname = copy.getRealname();
        this->_username = copy.getUsername();
        this->_hostname = copy.getHostname();
        this->_buffer = copy.getBuffer();
        this->_status = copy.getStatus();
    }
    return *this;
}

Client::~Client()
{
}