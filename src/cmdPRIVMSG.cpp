#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.3.1
// Command: PRIVMSG
// Parameters: <msgtarget> <text to be sent>
void Server::cmdPRIVMSG(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	// validar se tem target
	if (message->arguments[0].empty())
	{
		// reply ERR_NORECIPIENT
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NORECIPIENT + " " + client.nick + " PRIVMSG :No recipient given (PRIVMSG)\r\n");
		return;
	}
	// validar se tem conteudo
	if (message->arguments[1].empty())
	{
		// reply ERR_NOTEXTTOSEND
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NOTEXTTOSEND + " " + client.nick + " :No text to send\r\n");
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
						sendMessage(i->first->socket, std::string(":") + client.nick + "!" + client.userAtHost + " PRIVMSG " + channel->channelName + " :" + message->arguments[1] + "\r\n");
				}
				return;
			}
		}
		// reply ERR_CANNOTSENDTOCHAN
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_CANNOTSENDTOCHAN + " " + client.nick + " " + message->arguments[0] + " :Cannot send to channel\r\n");
		return;
	}
	// se enviar para outro user
	else
	{
		Client* recipient = getClientByNick(message->arguments[0]);
		if (!recipient)
		{
			// reply ERR_NOSUCHNICK
			sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NOSUCHNICK + " " + client.nick + " " + message->arguments[0] + " :No such nick/channel\r\n");
			return;
		}
		// enviar mensagem para o user
		sendMessage(recipient->socket, std::string(":") + client.nick + " PRIVMSG " + recipient->nick + " :" + message->arguments[1] + "\r\n");
		return;
	}
}
