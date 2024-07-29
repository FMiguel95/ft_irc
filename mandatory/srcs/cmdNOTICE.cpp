#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.3.1
// Command: PRIVMSG
// Parameters: <msgtarget> <text to be sent>
void Server::cmdNOTICE(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	// validar se tem target
	if (message->arguments[0].empty())
	{
		return;
	}
	// validar se tem conteudo
	if (message->arguments[1].empty())
	{
		return;
	}
	// se enviar para um canal
	if (message->arguments[0][0] == '#' || message->arguments[0][0] == '&')
	{
		Channel* channel = getChannelByName(message->arguments[0]);
		if (channel)
		{
			std::map<Client*,char>::iterator userInChannel = channel->getClientInChannel(client.nick);
			if (userInChannel != channel->userList.end())
			{
				// enviar mensagem para todos no canal
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
	// se enviar para outro user
	else
	{
		Client* recipient = getClientByNick(message->arguments[0]);
		if (!recipient)
		{
			return;
		}
		// enviar mensagem para o user
		sendMessage(recipient->socket, std::string(":") + client.nick + "!" + client.userAtHost  + " NOTICE " + recipient->nick + " :" + message->arguments[1] + "\r\n");
		return;
	}
}
