#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.6.1
// Command: WHO
// Parameters: [ <mask> [ "o" ] ]
void Server::cmdWHO(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);
	
	// validar se o user esta registado
	if (!client.isRegistered)
		return;
	
	if (message->arguments[0].empty())
	{
		// implementar quando nao tem parametros - listar todos os users que nao tem canal em comum com o user
		sendMessage(socket, std::string(":") + SERVER_NAME " " + RPL_ENDOFWHO + " " + client.nick + " :End of /WHO list\r\n");
		return;
	}

	if (message->arguments[0][0] == '#' || message->arguments[0][0] == '&')
	{
		Channel* channel = getChannelByName(message->arguments[0]);
		if (channel)
		{
			// for each user in the channel reply RPL_WHOREPLY
			for (std::map<Client *, char>::iterator it = channel->userList.begin(); it != channel->userList.end(); ++it)
			{
				sendMessage(socket, std::string(":") + SERVER_NAME " " + RPL_WHOREPLY + " " + client.nick + " " + channel->channelName + " " + it->first->user + " " + it->first->hostname + " localhost " + it->first->nick + " H :0 " + it->first->realname + "\r\n");
			}
		}
	}
	else // procurar nickname
	{
		Client* user = getClientByNick(message->arguments[0]);
		if (user)
		{
			// reply RPL_WHOREPLY
			sendMessage(socket, std::string(":") + SERVER_NAME " " + RPL_WHOREPLY + " " + client.nick + " * " + user->nick + " " + user->user + " " + user->hostname + " localhost " + user->nick + " H :0 " + user->realname + "\r\n");
		}
	}
	// reply RPL_ENDOFWHO
	sendMessage(socket, std::string(":") + SERVER_NAME " " + RPL_ENDOFWHO + " " + client.nick + " :End of /WHO list\r\n");
}
