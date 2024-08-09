#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.7
// Command: INVITE
// Parameters: Parameters: <nickname> <channel>
void Server::cmdINVITE(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the client is registered
	if (!client.isRegistered)
		return;

	// Validate that the message has enough parameters
	if (message->arguments[0].empty() || message->arguments[1].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " INVITE :Not enough parameters\r\n");
		return;
	}

	// Validate that the invited nick exists
	Client *invitedClient = getClientByNick(message->arguments[0]);
	if (!invitedClient)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHNICK + " " + client.nick + " " + message->arguments[0] + " :No such nick/channel\r\n");
		return;
	}

	// Validate that the channel exists
	Channel *channel = getChannelByName(message->arguments[1]);
	if (!channel)
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + message->arguments[1] + " :No such channel\r\n");
		return;
	}

	// Validate that the client is in the channel
	std::map<Client *, char>::iterator it;
	for (it = channel->userList.begin(); it != channel->userList.end(); ++it)
	{
		if (it->first->nick == client.nick)
			break;
	}
	if (it == channel->userList.end())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOTONCHANNEL + " " + client.nick + " " + message->arguments[1] + " :You're not on that channel\r\n");
		return;
	}

	// Validate that the client has the necessary permissions to invite someone to the channel
	if (channel->channelMode & MODE_i)
	{
		if (!(it->second & MODE_o))
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_CHANOPRIVSNEEDED + " " + client.nick + " " + message->arguments[1] + " :You're not channel operator\r\n");
			return;
		}
	}

	// Validate that the invited client is not already in the channel
	std::map<Client *, char>::iterator it1;
	for (it1 = channel->userList.begin(); it1 != channel->userList.end(); ++it1)
	{
		if (it1->first->nick == message->arguments[0])
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_USERONCHANNEL + " " + client.nick + " " + message->arguments[0] + " " + message->arguments[1] + " :is already on channel\r\n");
			return;
		}
	}

	// Tell the client that the invite was sent
	sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_INVITING + " " + client.nick + " " + message->arguments[0] + " " + message->arguments[1] + "\r\n");

	// Tell the invited nick that he was invited to a channel
	sendMessage(invitedClient->socket, std::string(":") + client.nick + "!" + client.userAtHost + " INVITE " + message->arguments[0] + " " + message->arguments[1] + "\r\n");

	// Add the invited client to the invited users list of the channel
	channel->invitedUsers.push_back(invitedClient);
}
