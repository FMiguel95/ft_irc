#include "../inc/Server.hpp"

void Server::addClientToChannel(Client& client, Channel& channel)
{
	if (channel.userList.empty())
		channel.userList.insert(std::pair<Client*,char>(&client, MODE_o));
	else
		channel.userList.insert(std::pair<Client*,char>(&client, 0));
	// notificar todos os users do canal
	for (std::map<Client*,char>::iterator i = channel.userList.begin(); i != channel.userList.end(); ++i)
		sendMessage(i->first->socket, std::string(":") + client.nick + "!" + client.user + " JOIN " + channel.channelName + "\r\n");
	// enviar o topico reply RPL_TOPIC ou RPL_NOTOPIC
	// talvez enviar tambem RPL_TOPICWHOTIME
	// enviar a lista de nomes -> reply RPL_NAMREPLY e RPL_ENDOFNAMES
}

void Server::attempJoin(Client& client, const std::string& channelName, const std::string& providedKey)
{
	//std::cout << channelName << std::endl;
	// validar se o nome nao contem caracteres incorretos
	if (Server::isChannelNameValid(channelName) == false)
	{
		// reply ERR_NOSUCHCHANNEL
		return;
	}
	Channel* channel = NULL;
	// verificar se o canal ja existe
	for (std::list<Channel>::iterator i = channels.begin(); i != channels.end(); ++i)
	{
		// se ja existir
		if (i->channelName == channelName)
		{
			channel = &(*i);
			// validar se o canal tem user limit e ja esta cheio
			if (channel->channelMode & MODE_l && channel->userList.size() >= channel->userLimit)
			{
				// reply ERR_CHANNELISFULL
				continue;
			}
			// validar se esta protegido por password e a key é correta
			if (channel->channelMode & MODE_k && channel->channelKey != providedKey)
			{
				// reply ERR_BADCHANNELKEY
				continue;
			}
			// validar se é invite only e o user nao foi convidado
			if (channel->channelMode & MODE_i
				&& std::find(channel->invitedUsers.begin(), channel->invitedUsers.end(), &client) == channel->invitedUsers.end())
			{
				// reply ERR_INVITEONLYCHAN
				continue;
			}
			// adicionar o user ao canal
			addClientToChannel(client, *channel);
			break;
		}
	}
	// if the channel wasnt found
	if (channel == NULL)
	{
		// create the channel
		Channel newChannel(channelName, "");
		channels.push_back(newChannel);
		channel = &channels.back();

		// adicionar o user ao canal
		addClientToChannel(client, *channel);
	}
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.1
// Command: JOIN
// Parameters: ( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0"
void Server::cmdJOIN(const int& socket, const t_message* message)
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

	std::vector<std::string> channelSplit;
	std::vector<std::string> keys;

	// splitting the channels with "," as delimiter
	size_t start = 0;
	size_t end;
	while ((end = message->arguments[0].find(",", start)) != std::string::npos)
	{
		channelSplit.push_back(message->arguments[0].substr(start, end - start));
		start = end + 1;
	}
	channelSplit.push_back(message->arguments[0].substr(start));

	if (!message->arguments[1].empty())
	{
		// splitting the keys with "," as delimiter
		start = 0;
		while ((end = message->arguments[1].find(",", start)) != std::string::npos)
		{
			keys.push_back(message->arguments[1].substr(start, end - start));
			start = end + 1;
		}
		keys.push_back(message->arguments[1].substr(start));
	}

	// for (size_t i = 0; i < channelSplit.size(); i++)
	// 	std::cout << channelSplit[i] << std::endl;
	// for (size_t i = 0; i < keys.size(); i++)
	// 	std::cout << keys[i] << std::endl;

	// for each channel the user is trying to join
	for (std::vector<std::string>::iterator i = channelSplit.begin(); i != channelSplit.end(); ++i)
	{
		if (i - channelSplit.begin() < keys.size())
			attempJoin(client, *i, keys[i - channelSplit.begin()]);
		else
			attempJoin(client, *i, "");
	}	
}
