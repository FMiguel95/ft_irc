#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.7
// Command: QUIT
// Parameters: [ <Quit Message> ]
void Server::cmdQUIT(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	// Send a QUIT message to all channels the user is in
	std::string reason = message->arguments[0];
	if (reason.empty())
		reason = "Client Quit";
	sendMessage(socket, std::string(":") + client.nick + "!" + client.userAtHost + " QUIT :" + reason + "\r\n");

	// Unregister the client
	unregisterClient(client, reason);

	// Send a closing link message to the client
	sendMessage(socket, std::string(":") + _serverHostname + " ERROR :Closing link\r\n");
}
