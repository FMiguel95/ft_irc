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
			continue;
		}
		// validate the client is in the channel
		std::map<Client *, char>::iterator clientInChannel = channel->getClientInChannel(client.nick);
		if (clientInChannel == channel->userList.end())
		{
			// reply ERR_NOTONCHANNEL
			continue;
		}
		sendMessage(socket, std::string(":") + client.nick + "!" + client.userAtHost + " PART " + channel->channelName + " :" + message->arguments[1] + "\r\n");
		channel->userList.erase(clientInChannel);
		broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " PART " + channel->channelName + " :" + message->arguments[1] + "\r\n");
	}
}
