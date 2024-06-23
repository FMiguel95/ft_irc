#include "../inc/Server.hpp"

Server::Server() : port(6667), password("") {}

Server::Server(const int& port, const std::string& password) : port(port), password(password) {}

Server::Server(const Server& src) :
port(src.port),
password(src.password),
clients(src.clients),
channels(src.channels)
{}

Server::~Server() {}

Server& Server::operator =(const Server& src)
{
	if (this != &src)
	{
		port = src.port;
		password = src.password;
		clients = src.clients;
		channels = src.channels;
	}
	return *this;
}

int Server::run()
{
	return 0;
}
