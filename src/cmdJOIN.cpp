#include "../inc/Server.hpp"

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

	/*for (size_t i = 0; i < channelSplit.size(); i++)
		std::cout << channelSplit[i] << std::endl;
	for (size_t i = 0; i < keys.size(); i++)
		std::cout << keys[i] << std::endl;*/

	for (std::vector<std::string>::iterator i = channelSplit.begin(); i != channelSplit.end(); ++i)
	{
		// validar se o nome nao contem caracteres incorretos
		//std::cout << *i << std::endl;
		if (isChannelNameValid(*i) == false)
		{
			// reply ERR_NOSUCHCHANNEL
			continue;
		}
		Channel* channel = NULL;
		// verificar se o canal ja existe
		for (std::list<Channel>::iterator j = channels.begin(); j != channels.end(); ++j)
		{
			if (j->channelName == *i)
			{
				channel = &(*j);
				// validar se o canal tem user limit e ja esta cheio
				if (channel->channelMode & MODE_l && channel->userList.size() >= channel->userLimit)
				{
					// reply ERR_CHANNELISFULL
					continue;
				}
				// validar se esta protegido por password e a key é correta
				if (channel->channelMode & MODE_k && std::find(keys.begin(), keys.end(), channel->channelKey) == keys.end())
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
				channel->userList.insert(std::pair<Client*,char>(&client, 0));
				// enviar o topico
				// notificar todos os users do canal
				for (std::map<Client*,char>::iterator k = channel->userList.begin(); k != channel->userList.end(); ++k)
				{
					// TODO check hostmask
					sendMessage(k->first->socket, std::string(":") + client.nick + "!" + client.user + " JOIN " + channel->channelName + "\r\n");
				}
				//std::cout << "breaking" << std::endl;
				break;
			}
		}
		//std::cout << "good break" << std::endl;

		if (channel == NULL)
		{
			Channel newChannel(*i, "");
			channels.push_back(newChannel);
			channel = &channels.back();

			// CREATE NEW CHANNEL:
			// adicionar o user ao canal
			channel->userList.insert(std::pair<Client*,char>(&client, MODE_o));
			sendMessage(socket, ":" + client.nick + "!" + client.user + " JOIN :" + channel->channelName + "\r\n");
			// enviar o topico
		}
		
		// se nao existir, criar e adicionar o user como operator

		// if the JOIN is successful the server will reply with:
		// a JOIN message
		// the channels topic with RPL_TOPIC, or RPL_NOTOPIC if no topic is set
		// the list of users currently in the channel

		// notify the other users in the channel

	}
	//std::cout << "for loops exited" << std::endl;

	// validacoes de invites etcccccccccccccc
	
}
