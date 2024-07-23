#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.7
// Command: QUIT
// Parameters: [ <Quit Message> ]
void Server::cmdQUIT(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	std::string reason = message->arguments[0];
	if (reason.empty())
		reason = "Client Quit";
	sendMessage(socket, std::string(":") + client.nick + "!" + client.userAtHost + " QUIT :" + reason + "\r\n");
	unregisterClient(client, reason);
	sendMessage(socket, std::string(":") + SERVER_ADDRESS + " ERROR :Closing link\r\n");
}
