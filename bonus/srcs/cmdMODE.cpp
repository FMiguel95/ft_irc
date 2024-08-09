#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.3
// Command: MODE
// Parameters: <channel> *( ( "-" / "+" ) *<modes> *<modeparams> )
// Parameters: <nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )
void Server::cmdMODE(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the client is registered
	if (!client.isRegistered)
		return;

	// Validate that there are enough parameters
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " MODE :Not enough parameters\r\n");
		return;
	}

	// If the first parameter is a channel
	if (message->arguments[0][0] == '#' || message->arguments[0][0] == '&')
	{
		// Validate that the channel exists
		Channel* channel = getChannelByName(message->arguments[0]);
		if (!channel)
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + message->arguments[0] + " :No such channel\r\n");
			return;
		}
		
		// If there are no more parameters, show the channel modes
		if (message->arguments[1].empty())
		{
			std::string reply;
			reply = std::string(":") + _serverHostname + " " + RPL_CHANNELMODEIS + " " + client.nick + " " + channel->channelName + " ";
			if (channel->channelMode & MODE_i)
				reply += "i";
			if (channel->channelMode & MODE_t)
				reply += "t";
			if (channel->channelMode & MODE_k)
				reply += "k";
			if (channel->channelMode & MODE_l)
				reply += "l";
			reply += "\r\n";
			sendMessage(socket, reply);
			return;
		}
		
		// Validate that the client is in the channel
		std::map<Client*,char>::iterator i = channel->getClientInChannel(client.nick);
		if (i == channel->userList.end() || (i->second & MODE_o) == 0)
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_CHANOPRIVSNEEDED + " " + client.nick + " " + channel->channelName + " :You're not channel operator\r\n");
			return;
		}

		// Set the requested modes
		bool setMode = true;
		int argIndex = 1;
		for (size_t i = 0; i < message->arguments[1].length(); i++)
		{
			switch (message->arguments[1][i])
			{
				case '+':
					setMode = true;
					break;
				case '-':
					setMode = false;
					break;
				case 'i':
					if (setMode && !(channel->channelMode & MODE_i))
					{
						channel->channelMode |= MODE_i;
						broadcastMessage(*channel, std::string(":") + client.nick + " MODE " + channel->channelName + " +i\r\n");
					}
					else if (!setMode && channel->channelMode & MODE_i)
					{
						channel->channelMode &= ~MODE_i;
						broadcastMessage(*channel, std::string(":") + client.nick + " MODE " + channel->channelName + " -i\r\n");
					}
					break;
				case 't':
					if (setMode && !(channel->channelMode & MODE_t))
					{
						channel->channelMode |= MODE_t;
						broadcastMessage(*channel, std::string(":") + client.nick + " MODE " + channel->channelName + " +t\r\n");
					}
					else if (!setMode && channel->channelMode & MODE_t)
					{
						channel->channelMode &= ~MODE_t;
						broadcastMessage(*channel, std::string(":") + client.nick + " MODE " + channel->channelName + " -t\r\n");
					}
					break;
				case 'k':
					argIndex++;
					if (setMode && argIndex < 15 && !(channel->channelMode & MODE_k))
					{
						std::string newKey = message->arguments[argIndex];
						if (newKey.empty())
							continue;
						if (newKey.find_first_of(",") != std::string::npos)
						{
							sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_INVALIDKEY + " " + client.nick + " " + channel->channelName + " :Invalid channel key\r\n");
							continue;
						}
						newKey.erase(std::remove(newKey.begin(), newKey.end(), ' '), newKey.end());
						channel->channelMode |= MODE_k;
						channel->channelKey = newKey;
						broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " MODE " + channel->channelName + " +k " + newKey + "\r\n");
					}
					else if (!setMode && channel->channelMode & MODE_k)
					{
						channel->channelMode &= ~MODE_k;
						broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " MODE " + channel->channelName + " -k\r\n");
					}
					break;
				case 'l':
					argIndex++;
					if (setMode && argIndex < 15 && !(channel->channelMode & MODE_l))
					{
						int newLimit = std::atoi(message->arguments[argIndex].c_str());
						if (newLimit <= 0)
							continue;
						channel->channelMode |= MODE_l;
						channel->userLimit = newLimit;
						std::ostringstream reply;
						reply << ":" << client.nick << "!" << client.userAtHost << " MODE " << channel->channelName << " +l " << newLimit << "\r\n";
						broadcastMessage(*channel, reply.str());
					}
					else if (!setMode && channel->channelMode & MODE_l)
					{
						channel->channelMode &= ~MODE_l;
						broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " MODE " + channel->channelName + " -l\r\n");
					}
					break;
				case 'o':
					argIndex++;
					if (setMode && argIndex < 15 && !message->arguments[argIndex].empty())
					{
						std::map<Client*,char>::iterator clientInChannel = channel->getClientInChannel(message->arguments[argIndex]); 
						if (clientInChannel == channel->userList.end())
						{
							sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_USERNOTINCHANNEL + " " + client.nick + " " + message->arguments[argIndex] + " " + channel->channelName + " :They aren't on that channel\r\n");
							continue;
						}
						if (clientInChannel->second & MODE_o)
							continue;
						clientInChannel->second |= MODE_o;
						broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " MODE " + channel->channelName + " +o " + message->arguments[argIndex] + "\r\n");
					}
					else if (!setMode)
					{
						std::map<Client*,char>::iterator clientInChannel = channel->getClientInChannel(message->arguments[argIndex]); 
						if (clientInChannel == channel->userList.end())
						{
							sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_USERNOTINCHANNEL + " " + client.nick + " " + message->arguments[argIndex] + " " + channel->channelName + " :They aren't on that channel\r\n");
							continue;
						}
						if (!(clientInChannel->second & MODE_o))
							continue;
						clientInChannel->second &= ~MODE_o;
						broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " MODE " + channel->channelName + " -o " + message->arguments[argIndex] + "\r\n");
					}
					break;
				default:
					sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_UMODEUNKNOWNFLAG + " " + client.nick + " :Unknown MODE flag\r\n");
					break;
			}
		}
	
	}
	// If the first parameter is a nickname
	else
	{
		// Validate that the user exists in the server
		Client* user = getClientByNick(message->arguments[0]);
		if (!user)
		{
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHNICK + " " + client.nick + " " + message->arguments[0] + " :No such nick/channel\r\n");
			return;
		}
		// If there are no more parameters, show the user modes
		if (message->arguments[1].empty())
		{
			std::string reply;
			reply = std::string(":") + _serverHostname + " " + RPL_UMODEIS + " " + client.nick + "\r\n";
			sendMessage(socket, reply);
			return;
		}
	}
}
