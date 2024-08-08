#include <iostream>
#include <csignal>
#include "../incs/Server.hpp"

static void handle_sigint(int signal)
{
	if (signal == SIGINT || signal == SIGHUP || signal == SIGTERM)
		Server::run = false;
}

static bool str2uint16(char* str, int* out)
{
	*out = 0;
	int i = 0;
	if (str == NULL || str[0] == '\0')
		return false;
	while (str[i])
	{
		if (!std::isdigit(str[i]))
			return false;
		*out = *out * 10 + str[i] - '0';
		if (*out > 0xFFFF)
			return false;
		i++;
	}
	return true;
}

int main(int ac, char** av)
{
	if (ac != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}

	int port;
	if (!str2uint16(av[1], &port))
	{
		std::cerr << "Invalid port" << std::endl;
		return 1;
	}
	std::string password = av[2];

	std::signal(SIGINT, handle_sigint);
	std::signal(SIGHUP, handle_sigint);
	std::signal(SIGTERM, handle_sigint);
	Server server(port, password);
	return server.runServer();
}
