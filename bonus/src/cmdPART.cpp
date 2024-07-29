#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.2
// Command: PART
// Parameters: <channel> *( "," <channel> ) [ <Part Message> ]
void Server::cmdPART(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	if (message->arguments[0].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " PART :Not enough parameters\r\n");
		return;
	}

	// splitting the channels with "," as delimiter
	std::vector<std::string> channelSplit;
	size_t start = 0;
	size_t end;
	while ((end = message->arguments[0].find(",", start)) != std::string::npos)
	{
		channelSplit.push_back(message->arguments[0].substr(start, end - start));
		start = end + 1;
	}
	channelSplit.push_back(message->arguments[0].substr(start));

	// run the request for each channel
	for (std::vector<std::string>::iterator i = channelSplit.begin(); i != channelSplit.end(); ++i)
	{
		Channel* channel = getChannelByName(*i);
		// validate the channel exists
		if (!channel)
		{
			// reply ERR_NOSUCHCHANNEL
			sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + *i + " :No such channel\r\n");
			continue;
		}
		// validate the client is in the channel
		std::map<Client *, char>::iterator clientInChannel = channel->getClientInChannel(client.nick);
		if (clientInChannel == channel->userList.end())
		{
			// reply ERR_NOTONCHANNEL
			sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NOTONCHANNEL + " " + client.nick + " " + *i + " :You're not on that channel\r\n");
			continue;
		}
		sendMessage(socket, std::string(":") + client.nick + "!" + client.userAtHost + " PART " + channel->channelName + " :" + message->arguments[1] + "\r\n");
		channel->userList.erase(clientInChannel);
		broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " PART " + channel->channelName + " :" + message->arguments[1] + "\r\n");
		
		// if the user had an invite for that channel, remove it
		for (std::list<Client*>::iterator i = channel->invitedUsers.begin(); i != channel->invitedUsers.end(); ++i)
		{
			if ((*i)->nick == client.nick)
			{
				channel->invitedUsers.erase(i);
				break;
			}
		}
		
		// if the channel is empty, remove it
		if (channel->userList.empty())
		{
			for (std::list<Channel>::iterator it = channels.begin(); it != channels.end(); ++it)
			{
				if (it->channelName == channel->channelName)
				{
					channels.erase(it);
					break;
				}
			}
		}
	}
}
