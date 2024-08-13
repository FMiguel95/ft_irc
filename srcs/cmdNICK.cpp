#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.2
// Command: NICK
// Parameters: <nickname>
void Server::cmdNICK(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the right amount of arguments were sent
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NONICKNAMEGIVEN + " " + client.nick + " :No nickname given\r\n");
		return;
	}

	// Validate that the nickname is valid
	if (isNickValid(message->arguments[0]) == false)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_ERRONEUSNICKNAME + " " + client.nick + " " + message->arguments[0] + " :Erroneous nickname\r\n");
		return;
	}

	// Validate that the nickname is not in use
	Client *clientInUse = getClientByNick(message->arguments[0]);
	if (clientInUse)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NICKNAMEINUSE + " " + client.nick + " " + message->arguments[0] + " :Nickname is already in use\r\n");
		return;
	}

	// If the the user is registered
	if (client.isRegistered)
	{
		// Broadcast the new nickname to all channels the user is in
		sendMessage(socket, std::string(":" + client.nick + " NICK " + message->arguments[0] + "\r\n"));
		for (std::list<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			std::map<Client *, char>::iterator user = it->getClientInChannel(client.nick);
			if (user != it->userList.end())
				broadcastMessage(*it, std::string(":" + client.nick + " NICK " + message->arguments[0] + "\r\n"));
		}
		client.nick = message->arguments[0];
		return;
	}
	// If the client is getting registered
	else
	{
		// Set the new nickname
		client.nick = message->arguments[0];
		client.nickOk = true;
		checkRegistration(client);
	}
}

bool Server::isNickValid(const std::string& nick)
{
	if (nick.length() > 63)
		return false;
	for (size_t i = 0; i < nick.size(); i++)
	{
		if (std::isalnum(nick[i]) == false && nick[i] != '\\' && nick[i] != '|' 
			&& nick[i] != '[' && nick[i] != ']' 
			&& nick[i] != '{' && nick[i] != '}'
			&& nick[i] != '_')
			return (false);
	}
	return (true);
}
