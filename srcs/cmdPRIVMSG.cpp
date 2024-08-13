#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.3.1
// Command: PRIVMSG
// Parameters: <msgtarget> <text to be sent>
void Server::cmdPRIVMSG(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate if the user is registered
	if (!client.isRegistered)
		return;

	// Validate that the message has a recipient
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NORECIPIENT + " " + client.nick + " PRIVMSG :No recipient given (PRIVMSG)\r\n");
		return;
	}

	// Validate that the message has text to send
	if (message->arguments[1].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOTEXTTOSEND + " " + client.nick + " :No text to send\r\n");
		return;
	}

	// If the recipient is a channel
	if (message->arguments[0][0] == '#' || message->arguments[0][0] == '&')
	{
		Channel* channel = getChannelByName(message->arguments[0]);
		if (channel)
		{
			// Send the message to all users in the channel
			std::map<Client*,char>::iterator userInChannel = channel->getClientInChannel(client.nick);
			if (userInChannel != channel->userList.end())
			{
				for (std::map<Client*,char>::iterator i = channel->userList.begin(); i != channel->userList.end(); ++i)
				{
					if (i->first->nick != client.nick)
						sendMessage(i->first->socket, std::string(":") + client.nick + "!" + client.userAtHost + " PRIVMSG " + channel->channelName + " :" + message->arguments[1] + "\r\n");
				}
				return;
			}
		}
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_CANNOTSENDTOCHAN + " " + client.nick + " " + message->arguments[0] + " :Cannot send to channel\r\n");
		return;
	}
	// If the recipient is a user
	else
	{
		// Validate that the recipient exists
		Client* recipient = getClientByNick(message->arguments[0]);
		if (!recipient)
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHNICK + " " + client.nick + " " + message->arguments[0] + " :No such nick/channel\r\n");
			return;
		}
		// Send the message to the recipient
		sendMessage(recipient->socket, std::string(":") + client.nick + " PRIVMSG " + recipient->nick + " :" + message->arguments[1] + "\r\n");
		return;
	}
}
