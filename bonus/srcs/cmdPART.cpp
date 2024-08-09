#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.2
// Command: PART
// Parameters: <channel> *( "," <channel> ) [ <Part Message> ]
void Server::cmdPART(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the client is registered
	if (!client.isRegistered)
		return;

	// Validate that the message has enough parameters
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " PART :Not enough parameters\r\n");
		return;
	}

	// Split the channels with "," as delimiter
	std::vector<std::string> channelSplit;
	size_t start = 0;
	size_t end;
	while ((end = message->arguments[0].find(",", start)) != std::string::npos)
	{
		channelSplit.push_back(message->arguments[0].substr(start, end - start));
		start = end + 1;
	}
	channelSplit.push_back(message->arguments[0].substr(start));

	// Run the command for each channel
	for (std::vector<std::string>::iterator i = channelSplit.begin(); i != channelSplit.end(); ++i)
	{
		// Validate that the channel exists
		Channel* channel = getChannelByName(*i);
		if (!channel)
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + *i + " :No such channel\r\n");
			continue;
		}
		
		// Validate the client is in the channel
		std::map<Client *, char>::iterator clientInChannel = channel->getClientInChannel(client.nick);
		if (clientInChannel == channel->userList.end())
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOTONCHANNEL + " " + client.nick + " " + *i + " :You're not on that channel\r\n");
			continue;
		}
		
		// Send the PART message to the client and broadcast it to the channel
		sendMessage(socket, std::string(":") + client.nick + "!" + client.userAtHost + " PART " + channel->channelName + " :" + message->arguments[1] + "\r\n");
		channel->userList.erase(clientInChannel);
		broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " PART " + channel->channelName + " :" + message->arguments[1] + "\r\n");
		
		// If the user had an invite for that channel, remove it
		for (std::list<Client*>::iterator i = channel->invitedUsers.begin(); i != channel->invitedUsers.end(); ++i)
		{
			if ((*i)->nick == client.nick)
			{
				channel->invitedUsers.erase(i);
				break;
			}
		}
		
		// If the channel is empty, remove it
		if (channel->userList.empty())
		{
			for (std::list<Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
			{
				if (it->channelName == channel->channelName)
				{
					_channels.erase(it);
					break;
				}
			}
		}
	}
}
