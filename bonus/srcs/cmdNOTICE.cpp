#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.3.2
// Command: NOTICE
// Parameters: <msgtarget> <text>
void Server::cmdNOTICE(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the client is registered
	if (!client.isRegistered)
		return;

	// Validate that the message has enough parameters
	if (message->arguments[0].empty() || message->arguments[1].empty())
		return;
	
	// If sending to a channel
	if (message->arguments[0][0] == '#' || message->arguments[0][0] == '&')
	{
		// Send the message to all users in the channel
		Channel* channel = getChannelByName(message->arguments[0]);
		if (channel)
		{
			std::map<Client*,char>::iterator userInChannel = channel->getClientInChannel(client.nick);
			if (userInChannel != channel->userList.end())
			{
				for (std::map<Client*,char>::iterator i = channel->userList.begin(); i != channel->userList.end(); ++i)
				{
					if (i->first->nick != client.nick)
						sendMessage(i->first->socket, std::string(":") + client.nick + "!" + client.userAtHost + " NOTICE " + channel->channelName + " :" + message->arguments[1] + "\r\n");
				}
				return;
			}
		}
		return;
	}
	// If sending to a user
	else
	{
		// Validate that the recipient exists
		Client* recipient = getClientByNick(message->arguments[0]);
		if (!recipient)
			return;
		// Send the message to the recipient
		sendMessage(recipient->socket, std::string(":") + client.nick + "!" + client.userAtHost  + " NOTICE " + recipient->nick + " :" + message->arguments[1] + "\r\n");
		return;
	}
}
