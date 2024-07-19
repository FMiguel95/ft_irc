#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.4
// Command: TOPIC
// Parameters: <channel> [ <topic> ]
void Server::cmdTOPIC(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	// validar parametros
	if (message->arguments[0].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		return;
	}

	Channel* channel = getChannelByName(message->arguments[0]);
	// validar que o canal existe
	if (!channel)
	{
		// reply ERR_NOSUCHCHANNEL
		return;
	}

	std::map<Client*,char>::iterator userInChannel = channel->getClientInChannel(client.nick);
	// validar que o user está no canal
	if (userInChannel == channel->userList.end())
	{
		// reply ERR_NOTONCHANNEL
		return;
	}

	// se nao tiver segundo argumento apenas enviar o topico do canal
	if (message->arguments[1].empty() && message->raw[message->raw.length() - 1] != ':')
	{
		//std::cout << "empty topic:" << message->raw[message->raw.length() - 1] << std::endl;
		// reply RPL_TOPIC ou RPL_NOTOPIC se não existir
		if (channel->topic.empty())
			sendMessage(socket, std::string(":") + SERVER_NAME + " " + RPL_NOTOPIC + " " + client.nick + " " + channel->channelName + " :No topic is set\r\n");
		else
			sendMessage(socket, std::string(":") + SERVER_NAME + " " + RPL_TOPIC + " " + client.nick + " " + channel->channelName + " :" + channel->topic + "\r\n");
		return;
	}

	// se canal tiver modo +t e user nao for operator
	if (channel->channelMode & MODE_t && !(userInChannel->second & MODE_o))
	{
		// reply ERR_CHANOPRIVSNEEDED
		return;
	}

	// tudo OK, reply RPL_TOPIC ou RPL_NOTOPIC para todos os users no canal
	//std::cout << "change topic\n";
	channel->topic = message->arguments[1];
	if (channel->topic.empty())
		broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " TOPIC " + channel->channelName + " :\r\n");
	else
		broadcastMessage(*channel, std::string(":") + client.nick + "!" + client.userAtHost + " TOPIC " + channel->channelName + " :" + channel->topic + "\r\n");
}
