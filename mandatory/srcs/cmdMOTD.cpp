#include "../incs/Server.hpp"

void Server::sendMOTD(const Client& client)
{
	if (_hasMOTD == false)
	{
		// reply ERR_NOMOTD
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + ERR_NOMOTD + " " + client.nick + " :MOTD File is missing\n\r");
		return;
	}

	sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_MOTDSTART + " " + client.nick + " :- " + _serverHostname + " Message of the day - \n\r");

	std::istringstream motdStream(_messageOfTheDay);
	std::string line;
	while (std::getline(motdStream, line) )
	{
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_MOTD + " " + client.nick + " :" + line + "\n\r");
	}

	sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_ENDOFMOTD + " " + client.nick + " :End of /MOTD command.\n\r");
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.4.1
// Command: MOTD
// Parameters: [ <target> ]
void Server::cmdMOTD(const int& socket, const t_message* message)
{
	(void)message;
	Client& client = _clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;
	
	sendMOTD(client);
}
