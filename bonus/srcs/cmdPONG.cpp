#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.7.3
// Command: PONG
// Parameters: <server1> [ <server2> ]
void Server::cmdPONG(const int& socket, const t_message* message)
{
	(void)message;
	Client& client = clients.at(socket);

	// validate if the user is registered
	if (!client.isRegistered)
		return;

	client.pendingPong = false;
}
