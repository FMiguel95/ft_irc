#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.8
// Command: KICK
// Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
void Server::cmdKICK(const int& socket, const t_message* message)
{
	// obs.: HORSE says one channel, multiple users. RFC 2812 says multiple channels, multiple users. I went with the HORSE version.

	Client& client = _clients.at(socket);

	// Validate that the client is registered
	if (!client.isRegistered)
		return;

	// Validate that the message has enough parameters
	if (message->arguments[0].empty() || message->arguments[1].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " KICK :Not enough parameters\r\n");
		return;
	}

	// Validate that the channel exists
	Channel *channel = getChannelByName(message->arguments[0]);
	if (channel == NULL)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + message->arguments[0] + " :No such channel\r\n");
		return;
	}

	// Validate that the client is in the channel
	std::map<Client*,char>::iterator clientInChannel = channel->getClientInChannel(client.nick);
	if (clientInChannel == channel->userList.end())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOTONCHANNEL + " " + client.nick + " " + message->arguments[0] + " :You're not on that channel\r\n");
		return;
	}
	
	// Validate that the client is a channel operator
	if (!(clientInChannel->second & MODE_o))
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_CHANOPRIVSNEEDED + " " + client.nick + " " + message->arguments[0] + " :You're not channel operator\r\n");	
		return;
	}

	// Split the users with "," as delimiter
	std::vector<std::string> userSplit;
	size_t start = 0;
	size_t end;
	while ((end = message->arguments[1].find(",", start)) != std::string::npos)
	{
		userSplit.push_back(message->arguments[1].substr(start, end - start));
		start = end + 1;
	}
	userSplit.push_back(message->arguments[1].substr(start));

	// Validate that the users being kicked exist in the server and are in the channel
	Client *target;
	std::map<Client*,char>::iterator targetInChannel;
	for (std::vector<std::string>::iterator it = userSplit.begin(); it != userSplit.end(); ++it)
	{
		target = getClientByNick(*it);
		if (target == NULL)
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_USERNOTINCHANNEL + " " + client.nick + " " + *it + " " + message->arguments[0] + " :They aren't on that channel\r\n");
			return;
		}
		targetInChannel = channel->getClientInChannel(target->nick);
		if (targetInChannel == channel->userList.end())
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_USERNOTINCHANNEL + " " + client.nick + " " + *it + " " + message->arguments[0] + " :They aren't on that channel\r\n");
			return;
		}
	}

	// Check if there is a comment for the KICK, if not, use the client's nick
	std::string comment;
	if (message->arguments[2].empty())
		comment = client.nick;
	else
		comment = message->arguments[2]; 

	// Notify all users in the channel that the user was kicked
	broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " KICK " + channel->channelName + " " + message->arguments[1] + " :" + comment + "\r\n");
	
	// Remove the user from the channel
	channel->userList.erase(targetInChannel);

	// If the user had an invite for that channel, remove it
	for (std::list<Client*>::iterator i = channel->invitedUsers.begin(); i != channel->invitedUsers.end(); ++i)
	{
		if ((*i)->nick == client.nick)
		{
			channel->invitedUsers.erase(i);
			break;
		}
	}
	
	// If the channel has now 0 users, remove it
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
