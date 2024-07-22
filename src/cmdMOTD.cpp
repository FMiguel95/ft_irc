#include "../inc/Server.hpp"

void Server::sendMOTD(const Client& client)
{
	if (messageOfTheDay.empty())
	{
		// reply ERR_NOMOTD
		sendMessage(client.socket, std::string(":") + SERVER_NAME + " " + ERR_NOMOTD + " " + client.nick + " :MOTD File is missing\n\r");
		return;
	}

	sendMessage(client.socket, std::string(":") + SERVER_NAME + " " + RPL_MOTDSTART + " " + client.nick + " :- " + SERVER_NAME + " Message of the day - \n\r");

	std::istringstream motdStream(messageOfTheDay);
	std::string line;
	while (std::getline(motdStream, line) )
	{
		sendMessage(client.socket, std::string(":") + SERVER_NAME + " " + RPL_MOTD + " " + client.nick + " :" + line + "\n\r");
	}

	sendMessage(client.socket, std::string(":") + SERVER_NAME + " " + RPL_ENDOFMOTD + " " + client.nick + " :End of /MOTD command.\n\r");
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.4.1
// Command: MOTD
// Parameters: [ <target> ]
void Server::cmdMOTD(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;
	
	sendMOTD(client);
}
