#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.8
// Command: KICK
// Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
void Server::cmdKICK(const int& socket, const t_message* message)
{
	// obs.: HORSE says one channel, multiple users. RFC 2812 says multiple channels, multiple users. I went with the HORSE version.

	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	// validar se tem 2 parametros ERR_NEEDMOREPARAMS
	if (message->arguments[0].empty() || message->arguments[1].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " KICK :Not enough parameters\r\n");
		return;
	}

	// se o canal não existir ERR_NOSUCHCHANNEL
	Channel *channel = getChannelByName(message->arguments[0]);
	if (channel == NULL)
	{
		// reply ERR_NOSUCHCHANNEL
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + message->arguments[0] + " :No such channel\r\n");
		return;
	}

	// ERR_NOTONCHANNEL: se o client não estiver no canal
	std::map<Client*,char>::iterator clientInChannel = channel->getClientInChannel(client.nick);
	if (clientInChannel == channel->userList.end())
	{
		// reply ERR_NOTONCHANNEL
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NOTONCHANNEL + " " + client.nick + " " + message->arguments[0] + " :You're not on that channel\r\n");
		return;
	}
	
	// ERR_CHANOPRIVSNEEDED: se o client não for operador
	if (!(clientInChannel->second & MODE_o))
	{
		// reply ERR_CHANOPRIVSNEEDED
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_CHANOPRIVSNEEDED + " " + client.nick + " " + message->arguments[0] + " :You're not channel operator\r\n");	
		return;
	}

	// splitting the users with "," as delimiter
	std::vector<std::string> userSplit;
	
	size_t start = 0;
	size_t end;
	while ((end = message->arguments[1].find(",", start)) != std::string::npos)
	{
		userSplit.push_back(message->arguments[1].substr(start, end - start));
		start = end + 1;
	}
	userSplit.push_back(message->arguments[1].substr(start));

	// ERR_USERNOTINCHANNEL: se o <user> não estiver no canal
	Client *target;
	std::map<Client*,char>::iterator targetInChannel;
	for (std::vector<std::string>::iterator it = userSplit.begin(); it != userSplit.end(); ++it)
	{
		std::cout << *it << std::endl;
		target = getClientByNick(*it);
		if (target == NULL)
		{
			std::cout << "RPL ERR_USERNOTINCHANNEL" << std::endl;
			// reply ERR_USERNOTINCHANNEL
			sendMessage(socket, std::string(":") + serverHostname + " " + ERR_USERNOTINCHANNEL + " " + client.nick + " " + *it + " " + message->arguments[0] + " :They aren't on that channel\r\n");
			return;
		}
		targetInChannel = channel->getClientInChannel(target->nick);
		if (targetInChannel == channel->userList.end())
		{
			std::cout << "RPL ERR_USERNOTINCHANNEL" << std::endl;
			// reply ERR_USERNOTINCHANNEL
			sendMessage(socket, std::string(":") + serverHostname + " " + ERR_USERNOTINCHANNEL + " " + client.nick + " " + *it + " " + message->arguments[0] + " :They aren't on that channel\r\n");
			return;
		}
	}

	// if successfull:

	// define the comment
	std::string comment;
	if (message->arguments[2].empty())
		comment = client.nick;
	else
		comment = message->arguments[2]; 

	// broadcast KICK message to the channel
	broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " KICK " + channel->channelName + " " + message->arguments[1] + " :" + comment + "\r\n");
	
	// remove the user from the channel
	channel->userList.erase(targetInChannel);

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
