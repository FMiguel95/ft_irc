#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.7.2
// Command: PING
// Parameters: <server1> [ <server2> ]
void Server::cmdPING(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the user is registered
	if (!client.isRegistered)
		return;

	// Validar that the message has enough parameters
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOORIGIN + " " + client.nick + " PING :No origin specified\r\n");
		return;
	}

	// Send a PONG message back to the origin
	sendMessage(socket, std::string(":") + _serverHostname + " PONG " + _serverHostname + " :" + message->arguments[0] + "\r\n");
}
