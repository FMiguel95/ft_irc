#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.6.2
// Command: WHOIS
// Parameters: [ <target> ] <mask> *( "," <mask> )
void Server::cmdWHOIS(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);
	
	// Validate that the client is registered
	if (!client.isRegistered)
		return;
	
	// Validate that the message has enough parameters
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NONICKNAMEGIVEN + " " + client.nick + " :No nickname given\r\n");
		return;
	}

	// Validate that the target exists
	int index = 0;
	Client *target = getClientByNick(message->arguments[0]);
	if (!message->arguments[1].empty())	
	{
		index = 1;
		target = getClientByNick(message->arguments[1]);
	}
	if (target == NULL)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHNICK + " " + client.nick + " " + message->arguments[index] + " :No such nick/channel\r\n");
		return;
	}

	// Reply with the user information
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOISUSER + " " + client.nick + " " + target->nick + " " + target->user + " " + target->hostname + " * :" + target->realname + "\r\n");
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOISSERVER + " " + client.nick + " " + target->nick + " " + _serverHostname + " :A very cool server\r\n");
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOISHOST + " " + client.nick + " " + target->nick + " :is connecting from " + target->userAtHost + "\r\n");
	std::string channelList;
	for (std::list<Channel>::iterator it1 = _channels.begin(); it1 != _channels.end(); ++it1)
	{
		for (std::map<Client*,char>::iterator it2 = it1->userList.begin(); it2 != it1->userList.end(); ++it2)
		{
			if (it2->first == target)
				channelList += it1->channelName + " ";
		}
	}
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOISCHANNELS + " " + client.nick + " " + target->nick + " :" + channelList + "\r\n");
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_ENDOFWHOIS + " " + client.nick + " " + target->nick + " :End of WHOIS list\r\n");
}

