#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.3
// Command: MODE
// Parameters: <channel> *( ( "-" / "+" ) *<modes> *<modeparams> )
// Parameters: <nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )
void Server::cmdMODE(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	// se nao tem parametros, reply ERR_NEEDMOREPARAMS
	if (message->arguments[0].empty())
	{
		sendMessage(socket, std::string(":") + SERVER_NAME " " + ERR_NEEDMOREPARAMS + " " + client.nick + " MODE :Not enough parameters\r\n");
		return;
	}

	// se o primeiro parametro é um canal
	if (message->arguments[0][0] == '#' || message->arguments[0][0] == '&')
	{
		Channel* channel = getChannelByName(message->arguments[0]);
		if (!channel)
		{
			// reply ERR_NOSUCHCHANNEL
			sendMessage(socket, std::string(":") + SERVER_NAME " " + ERR_NOSUCHCHANNEL + " " + client.nick + " " + message->arguments[0] + " :No such channel\r\n");
			return;
		}
		// se nao tem mais parametros, reply RPL_CHANNELMODEIS
		if (message->arguments[1].empty())
		{
			std::string reply;
			reply = std::string(":") + SERVER_NAME " " + RPL_CHANNELMODEIS + " " + client.nick + " " + channel->channelName + " ";
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
		// ver se o user tem permissao para alterar os modos do canal
		std::map<Client*,char>::iterator i = channel->getClientInChannel(client.nick);
		if (i == channel->userList.end() || (i->second & MODE_o) == 0)
		{
			// reply ERR_CHANOPRIVSNEEDED
			sendMessage(socket, std::string(":") + SERVER_NAME " " + ERR_CHANOPRIVSNEEDED + " " + client.nick + " " + channel->channelName + " :You're not channel operator\r\n");
			return;
		}
		// iterar pelos modos
		bool setMode = true;
		int argIndex = -1;
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
				if (setMode)
				{
					channel->channelMode |= MODE_i;
					sendMessage(socket, std::string(":") + client.nick + " MODE " + channel->channelName + " +i\r\n");
				}
				else
				{
					channel->channelMode &= ~MODE_i;
					sendMessage(socket, std::string(":") + client.nick + " MODE " + channel->channelName + " -i\r\n");
				}
				break;
			case 't':
				if (setMode)
				{
					channel->channelMode |= MODE_t;
					sendMessage(socket, std::string(":") + client.nick + " MODE " + channel->channelName + " +t\r\n");
				}
				else
				{
					channel->channelMode &= ~MODE_t;
					sendMessage(socket, std::string(":") + client.nick + " MODE " + channel->channelName + " -t\r\n");
				}
				break;
			case 'k':
				argIndex++;
				break;
			case 'l':
				argIndex++;
				break;
			case 'o':
				argIndex++;
				break;
			default:
				// reply ERR_UMODEUNKNOWNFLAG
				sendMessage(socket, std::string(":") + SERVER_NAME " " + ERR_UMODEUNKNOWNFLAG + " " + client.nick + " :Unknown MODE flag\r\n");
				break;
			}
		}
	
	}
	else // procurar nickname
	{
		Client* user = getClientByNick(message->arguments[0]);
		if (!user)
		{
			// reply ERR_NOSUCHNICK
			return;
		}
		// se nao tem mais parametros, reply RPL_UMODEIS
		if (message->arguments[1].empty())
		{
			std::string reply;
			reply = std::string(":") + SERVER_NAME " " + RPL_UMODEIS + " " + client.nick + "\r\n";
			// no user modes supported
			// +o is different from channel to channel
			sendMessage(socket, reply);
			return;
		}
		else
		{
			// no user modes to add/remove 🫠
		}
	}
}
