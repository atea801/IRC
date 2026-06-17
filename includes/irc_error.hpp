#pragma once

enum IrcError {
    IRC_OK                  = 0,
    ERR_EMPTY               = 431,
    ERR_INVALID             = 432,
    ERR_NICK_TAKEN          = 433,

    ERR_NBR_PARAM           = 461,
    ERR_ALREADYREGISTERED   = 462,
    ERR_NOTREGISTERED       = 451,
    ERR_UNKNOWNCOMMAND      = 421,
};