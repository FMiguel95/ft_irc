#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.7.2
// Command: PING
// Parameters: <server1> [ <server2> ]

void Server::cmdPING(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validate if the user is registered
	if (!client.isRegistered)
		return;

	// validar se tem argumentos
	if (message->arguments[0].empty())
	{
		// reply ERR_NOORIGIN
		sendMessage(socket, std::string(":") + SERVER_ADDRESS + " " + ERR_NOORIGIN + " " + client.nick + " PING :No origin specified\r\n");
		return;
	}

	// reply PONG
	sendMessage(socket, std::string(":") + SERVER_ADDRESS + " PONG " + SERVER_ADDRESS + " :" + message->arguments[0] + "\r\n");
}