#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.7
// Command: INVITE
// Parameters: <nickname> <channel>
void Server::cmdINVITE(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	// validar parametros
	if (message->arguments[0].empty() || message->arguments[1].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		sendMessage(socket, std::string(":") + SERVER_ADDRESS + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " INVITE :Not enough parameters\r\n");
		return;
	}

	// validar que o nick existe reply ERR_NOSUCHNICK
	Client *invitedClient = getClientByNick(message->arguments[0]);
	if (!invitedClient)
	{
		// reply ERR_NOSUCHNICK
		sendMessage(socket, std::string(":") + SERVER_ADDRESS + " " + ERR_NOSUCHNICK + " " + client.nick + " " + message->arguments[0] + " :No such nick/channel\r\n");
		return;
	}

	// se o canal existir, validar que o user esta no canal reply ERR_NOTONCHANNEL
	Channel *channel = getChannelByName(message->arguments[1]);
	if (!channel)
	{
		// reply ERR_NOSUCHCHANNEL
		sendMessage(socket, std::string(":") + SERVER_ADDRESS + " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + message->arguments[1] + " :No such channel\r\n");
		return;
	}

	// validar que o user esta no canal
	std::map<Client *, char>::iterator it;
	for (it = channel->userList.begin(); it != channel->userList.end(); ++it)
	{
		if (it->first->nick == client.nick)
			break;
	}
	if (it == channel->userList.end())
	{
		// reply ERR_NOTONCHANNEL
		sendMessage(socket, std::string(":") + SERVER_ADDRESS + " " + ERR_NOTONCHANNEL + " " + client.nick + " " + message->arguments[1] + " :You're not on that channel\r\n");
		return;
	}

	// validar que o user tem permissao para convidar
	if (channel->channelMode & MODE_i)
	{
		if (!(it->second & MODE_o))
		{
			// reply ERR_CHANOPRIVSNEEDED
			sendMessage(socket, std::string(":") + SERVER_ADDRESS + " " + ERR_CHANOPRIVSNEEDED + " " + client.nick + " " + message->arguments[1] + " :You're not channel operator\r\n");
			return;
		}
	}

	// validar que o recipient nao esta no canal ainda: reply ERR_USERONCHANNEL
	std::map<Client *, char>::iterator it1;
	for (it1 = channel->userList.begin(); it1 != channel->userList.end(); ++it1)
	{
		if (it1->first->nick == message->arguments[0])
		{
			// reply ERR_USERONCHANNEL
			sendMessage(socket, std::string(":") + SERVER_ADDRESS + " " + ERR_USERONCHANNEL + " " + client.nick + " " + message->arguments[0] + " " + message->arguments[1] + " :is already on channel\r\n");
			return;
		}
	}

	// se OK reply RPL_INVITING
	sendMessage(client.socket, std::string(":") + SERVER_ADDRESS + " " + RPL_INVITING + " " + client.nick + " " + message->arguments[0] + " " + message->arguments[1] + "\r\n");

	// enviar convite ao user
	sendMessage(invitedClient->socket, std::string(":") + client.nick + "!" + client.userAtHost + " INVITE " + message->arguments[0] + " " + message->arguments[1] + "\r\n");

	// adicionar o user à lista de convidados
	channel->invitedUsers.push_back(invitedClient);
}
