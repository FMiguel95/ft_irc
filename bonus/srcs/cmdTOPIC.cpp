#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.4
// Command: TOPIC
// Parameters: <channel> [ <topic> ]
void Server::cmdTOPIC(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the user is registered
	if (!client.isRegistered)
		return;

	// Validate that the message has enough parameters
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " TOPIC :Not enough parameters\r\n");
		return;
	}

	// Validate that the channel exists
	Channel* channel = getChannelByName(message->arguments[0]);
	if (!channel)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + message->arguments[0] + " :No such channel\r\n");
		return;
	}

	// Validate that the user is in the channel
	std::map<Client*,char>::iterator userInChannel = channel->getClientInChannel(client.nick);
	if (userInChannel == channel->userList.end())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOTONCHANNEL + " " + client.nick + " " + message->arguments[0] + " :You're not on that channel\r\n");
		return;
	}

	// If there is no second argument, send the channel's topic
	if (message->arguments[1].empty() && message->raw[message->raw.length() - 1] != ':')
	{
		if (channel->topic.empty())
			sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_NOTOPIC + " " + client.nick + " " + channel->channelName + " :No topic is set\r\n");
		else
			sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_TOPIC + " " + client.nick + " " + channel->channelName + " :" + channel->topic + "\r\n");
		return;
	}

	// If the channel has mode +t, the user wanting to change the topic must be a channel operator
	if (channel->channelMode & MODE_t && !(userInChannel->second & MODE_o))
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_CHANOPRIVSNEEDED + " " + client.nick + " " + channel->channelName + " :You're not channel operator\r\n");
		return;
	}

	// Set the new topic
	channel->topic = message->arguments[1];
	if (channel->topic.empty())
		broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " TOPIC " + channel->channelName + " :\r\n");
	else
		broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " TOPIC " + channel->channelName + " :" + channel->topic + "\r\n");
}
