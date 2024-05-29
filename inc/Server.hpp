#pragma once

#include <map>
#include <list>
#include <algorithm>
#include <iostream>
#include "Client.hpp"
#include "Channel.hpp"

class Server
{
public:
	Server();
	Server(const Server& src);
	~Server();
	Server& operator =(const Server& src);

	int run(); // will be called in main

private:
	const int port;

	std::map<int,Client> clients;	// socket/client pair
	std::list<Channel> channels;	// list of channels
};
