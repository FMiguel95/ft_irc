#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.1
// Command: JOIN
// Parameters: ( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0"
void Server::cmdJOIN(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the client is registered
	if (!client.isRegistered)
		return;
	
	// Validate that the message has enough parameters
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " JOIN :Not enough parameters\r\n");
		return;
	}

	// converting channel names to lowercase
	std::string channels = message->arguments[0];
	for (int i = 0; channels[i]; i++)
		channels[i] = std::tolower(channels[i]);

	// Splitting the channels with "," as delimiter
	std::vector<std::string> channelSplit;
	std::vector<std::string> keys;
	size_t start = 0;
	size_t end;
	
	while ((end = channels.find(",", start)) != std::string::npos)
	{
		channelSplit.push_back(channels.substr(start, end - start));
		start = end + 1;
	}
	channelSplit.push_back(channels.substr(start));

	// If there are keys, split them with "," as delimiter
	if (!message->arguments[1].empty())
	{
		// Splitting the keys with "," as delimiter
		start = 0;
		while ((end = message->arguments[1].find(",", start)) != std::string::npos)
		{
			keys.push_back(message->arguments[1].substr(start, end - start));
			start = end + 1;
		}
		keys.push_back(message->arguments[1].substr(start));
	}

	// Try to join each channel
	for (std::vector<std::string>::iterator i = channelSplit.begin(); i != channelSplit.end(); ++i)
	{
		if (i - channelSplit.begin() < (long)keys.size())
			attempJoin(client, *i, keys[i - channelSplit.begin()]);
		else
			attempJoin(client, *i, "");
	}	
}

void Server::attempJoin(Client& client, const std::string& channelName, const std::string& providedKey)
{
	// Validate that the channel name doesn't contain any invalid characters
	if (Server::isChannelNameValid(channelName) == false)
	{
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + channelName + " :No such channel\r\n");
		return;
	}
	
	// Validation for existing channels
	Channel* channel = NULL;
	for (std::list<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
	{
		if (i->channelName == channelName)
		{
			channel = &(*i);
			// Validate that the user limit is not reached
			if (channel->channelMode & MODE_l && channel->userList.size() >= (size_t)channel->userLimit)
			{
				sendMessage(client.socket, std::string(":") + _serverHostname + " " + ERR_CHANNELISFULL + " " + client.nick + " " + channelName + " :Cannot join channel (+l)\r\n");
				continue;
			}
			// If it is password protected, validate that the key is correct
			if (channel->channelMode & MODE_k && channel->channelKey != providedKey)
			{
				sendMessage(client.socket, std::string(":") + _serverHostname + " " + ERR_BADCHANNELKEY + " " + client.nick + " " + channelName + " :Cannot join channel (+k)\r\n");
				continue;
			}
			// If it is invite only, validate that the user was invited
			if (channel->channelMode & MODE_i
				&& std::find(channel->invitedUsers.begin(), channel->invitedUsers.end(), &client) == channel->invitedUsers.end())
			{
				sendMessage(client.socket, std::string(":") + _serverHostname + " " + ERR_INVITEONLYCHAN + " " + client.nick + " " + channelName + " :Cannot join channel (+i)\r\n");
				continue;
			}
			// Add the user to the channel
			addClientToChannel(client, *channel);
			break;
		}
	}
	
	// For non-existing channels
	if (channel == NULL)
	{
		// Create the channel
		Channel newChannel(channelName, "");
		_channels.push_back(newChannel);
		channel = &_channels.back();

		// Add the user to the channel
		addClientToChannel(client, *channel);
	}
}

void Server::addClientToChannel(Client& client, Channel& channel)
{
	// If the channel is new, add the user as the operator, otherwise add the user as a regular user
	if (channel.userList.empty())
		channel.userList.insert(std::pair<Client*,char>(&client, MODE_o));
	else
		channel.userList.insert(std::pair<Client*,char>(&client, 0));

	//Notify all users in the channel that a new user has joined
	broadcastMessage(channel, std::string(":") + client.nick + "!" + client.userAtHost + " JOIN " + channel.channelName + "\r\n");
	
	// Send channel information (topic) to the user that joined
	if (channel.topic.empty())
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_NOTOPIC + " " + client.nick + " " + channel.channelName + " :No topic is set\r\n");
	else
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_TOPIC + " " + client.nick + " " + channel.channelName + " :" + channel.topic + "\r\n");
	
	// Send channel information (user list) to the user that joined
	std::string message = std::string(":") + _serverHostname + " " + RPL_NAMREPLY + " " + client.nick + " = " + channel.channelName + " :";
	for (std::map<Client*,char>::iterator i = channel.userList.begin(); i != channel.userList.end(); ++i)
	{
		std::string nextNickname;
		if (i->second & MODE_o)
			nextNickname += "@";
		nextNickname += i->first->nick;
		if (message.length() + nextNickname.length() > 510)	// test later
		{
			message += "\r\n";
			sendMessage(client.socket, message);
			message = std::string(":") + _serverHostname + " " + RPL_NAMREPLY + " " + client.nick + " = " + channel.channelName + " :";
		}
		message += nextNickname + " ";
	}
	message += "\r\n";
	sendMessage(client.socket, message);
	sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_ENDOFNAMES + " " + client.nick + " " + channel.channelName + " :End of /NAMES list.\r\n");
}
