#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.6.1
// Command: WHO
// Parameters: [ <mask> [ "o" ] ]
void Server::cmdWHO(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);
	
	// Validate that the client is registered
	if (!client.isRegistered)
		return;
	
	// If there are no parameters, list everyone on the server
	if (message->arguments[0].empty())
	{
		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOREPLY + " " + it->second.nick + " * " + it->second.user + " " + it->second.hostname + " " + _serverHostname + " " + it->second.nick + " H :0 " + it->second.realname + "\r\n");
		}
		sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_ENDOFWHO + " " + client.nick + " :End of /WHO list\r\n");
		return;
	}

	// If the mask is a channel, list everyone in the channel
	if (message->arguments[0][0] == '#' || message->arguments[0][0] == '&')
	{
		Channel* channel = getChannelByName(message->arguments[0]);
		if (channel)
		{
			for (std::map<Client *, char>::iterator it = channel->userList.begin(); it != channel->userList.end(); ++it)
				sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOREPLY + " " + client.nick + " " + channel->channelName + " " + it->first->user + " " + it->first->hostname + " " + _serverHostname + " " + it->first->nick + " H :0 " + it->first->realname + "\r\n");
		}
	}
	// If the mask is a user, list the user information
	else
	{
		Client* user = getClientByNick(message->arguments[0]);
		if (user)
			sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOREPLY + " " + client.nick + " * " + user->nick + " " + user->user + " " + user->hostname + " " + _serverHostname + " " + user->nick + " H :0 " + user->realname + "\r\n");
	}
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_ENDOFWHO + " " + client.nick + " " + message->arguments[0] + " :End of /WHO list\r\n");
}
