#pragma once

enum IrcError {
    IRC_OK                  = 0,
    ERR_NONICKNAMEGIVEN     = 431,
    ERR_ERRONEUSNICK        = 432,
    ERR_NICKNAMEINUSE       = 433,
    ERR_NEEDMOREPARAMS      = 461,
    ERR_ALREADYREGISTERED   = 462,
    ERR_NOTREGISTERED       = 451,
    ERR_UNKNOWNCOMMAND      = 421,
};