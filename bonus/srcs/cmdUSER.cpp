#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.3
// Command: USER
// Parameters: <user> <mode> <unused> <realname>
void Server::cmdUSER(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);
	
	// Validate that the client is not registered
	if (client.isRegistered)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_ALREADYREGISTRED + " " + client.nick + " :You may not reregister\r\n");
		return;
	}

	// Validate that the client has passed the password
	if (!_serverPassword.empty() && client.passOk == false)
		return;

	// Validate that the message has enough parameters
	if (message->arguments[0].empty() || message->arguments[1].empty() 
		|| message->arguments[2].empty() || message->arguments[3].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " USER :Not enough parameters\r\n");
		return;
	}

	// Set the client's user information
	client.user = std::string("~") + message->arguments[0];
	client.userAtHost = client.user + "@" + client.hostname;
	client.realname = message->arguments[3];
	client.userOk = true;

	checkRegistration(client);
}
