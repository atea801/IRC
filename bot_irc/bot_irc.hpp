#pragma once 

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <cctype>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <unistd.h>

enum SpamAction { 
	SPAM_NONE,
	SPAM_WARN,
	SPAM_KICK
};


const int SPAM_THRESHOLD = 4; // plus de 4 messages dans la fenetre = spam
const int SPAM_WINDOW = 5;    // duree de la fenetre, en secondes

struct UserState
{
    int msg_count;             // messages envoyes dans la fenetre courante
    std::time_t window_start;  // debut de la fenetre de comptage
    int warnings;              // nombre d'avertissements deja recus
    UserState() : msg_count(0), window_start(0), warnings(0) {}
};