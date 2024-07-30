#pragma once

#include "GameInstance.hpp"
#include <iostream>
#include <map>
#include <map>
#include <vector>
#include <list>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cctype>
#include <ctime>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define NICK "jack"
#define USER "jack 0 * :I'm a blackjack bot!"

class Bot
{
public:
	Bot(const std::string &serverAddress, int serverPort, const std::string &serverPassword);
	Bot(const Bot& src);
	~Bot();
	Bot& operator =(const Bot& src);

	int runBot();

private:
	Bot();
	std::map<std::string,GameInstance> _games; // list of games with client nickname as key
	std::string	_serverAddress;
	int			_serverPort;
	std::string	_serverPassword;
};
