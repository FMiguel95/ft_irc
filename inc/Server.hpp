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
	Server(int port, char** pass);
	Server(const Server& src);
	~Server();
	Server& operator =(const Server& src);

	int run(); // will be called in main

private:
	const int port;

	std::map<std::string,Client> clients;	// ip/client pair, chave tipo 127.0.0.1:12345
	std::list<Channel> channels;	// list of channels


	// funçao para validar o registo de um client
	// returns true if valid, else false
	// checks if the ip already exists in the clients list, then validates the parameters given (pass, nick, user, etc...)
	bool validateRegistration(std::string& ip, std::string& buffer);
};
