#pragma once

enum IrcError {
    IRC_OK                  = 0,
    ERR_NICK_EMPTY          = 431,
    ERR_NICK_INVALID        = 432,
    ERR_NICK_TAKEN          = 433,

    ERR_NEEDMOREPARAMS      = 461,
    ERR_ALREADYREGISTERED   = 462,
    ERR_NOTREGISTERED       = 451,
    ERR_UNKNOWNCOMMAND      = 421,
};