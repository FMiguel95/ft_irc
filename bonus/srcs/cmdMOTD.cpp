#include "../incs/Server.hpp"

void Server::sendMOTD(const Client& client)
{
	if (hasMOTD == false)
	{
		// reply ERR_NOMOTD
		sendMessage(client.socket, std::string(":") + serverHostname + " " + ERR_NOMOTD + " " + client.nick + " :MOTD File is missing\r\n");
		return;
	}

	sendMessage(client.socket, std::string(":") + serverHostname + " " + RPL_MOTDSTART + " " + client.nick + " :- " + serverHostname + " Message of the day - \r\n");

	std::istringstream motdStream(messageOfTheDay);
	std::string line;
	while (std::getline(motdStream, line) )
	{
		sendMessage(client.socket, std::string(":") + serverHostname + " " + RPL_MOTD + " " + client.nick + " :" + line + "\r\n");
	}

	sendMessage(client.socket, std::string(":") + serverHostname + " " + RPL_ENDOFMOTD + " " + client.nick + " :End of /MOTD command.\r\n");
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.4.1
// Command: MOTD
// Parameters: [ <target> ]
void Server::cmdMOTD(const int& socket, const t_message* message)
{
	(void)message;
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;
	
	sendMOTD(client);
}
