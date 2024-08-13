#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.1
// Command: PASS
// Parameters: <password>
void Server::cmdPASS(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);
	
	// Validate that the client is not registered
	if (client.isRegistered)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_ALREADYREGISTRED + " " + client.nick + " :You may not reregister\r\n");
		return; 
	}
	
	// If the server doesn't need a password, you can pass wherever password you want
	if (_serverPassword.empty())
	{
		client.passOk = true;
		return;
	}
	
	// Validate that the message has enough parameters
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " PASS :Not enough parameters\r\n");
		return; 
	}

	// Validate that the password passed matches the server password
	if (_serverPassword != message->arguments[0])
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_PASSWDMISMATCH + " " + client.nick + " :Password incorrect\r\n");
		return; 
	}

	// If the password is correct, set the passOk flag to true
	if (_serverPassword == message->arguments[0])
		client.passOk = true;
}

