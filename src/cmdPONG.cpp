#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.7.3
// Command: PONG
// Parameters: <server1> [ <server2> ]

void Server::cmdPONG(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validate if the user is registered
	if (!client.isRegistered)
		return;

	// reply PONG
	//sendMessage(socket, std::string(":") + SERVER_ADDRESS + " PONG " + SERVER_ADDRESS + " :" + message->arguments[0] + "\r\n");
	client.pendingPong = false;
}
