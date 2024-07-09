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
		sendMessage(socket, std::string(":localhost ") + ERR_NEEDMOREPARAMS + " " + client.nick + " MODE :Not enough parameters\r\n");
		return;
	}

	// se o primeiro parametro é um canal
	if (message->arguments[0][0] == '#' || message->arguments[0][0] == '&')
	{
		Channel* channel = getChannelByName(message->arguments[0]);
		if (!channel)
		{
			// reply ERR_NOSUCHCHANNEL
			sendMessage(socket, std::string(":localhost ") + ERR_NOSUCHCHANNEL + " " + client.nick + " " + message->arguments[0] + " :No such channel\r\n");
			return;
		}
		// se nao tem mais parametros, reply RPL_CHANNELMODEIS
		if (message->arguments[1].empty())
		{
			std::string reply;
			reply = std::string(":localhost ") + RPL_CHANNELMODEIS + " " + client.nick + " " + channel->channelName + " ";
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
		std::map<Client*,char>::const_iterator& i = channel->getClientInChannel(client.nick);
		if (i == channel->userList.end() || i->second & MODE_o == 0)
		{
			// reply ERR_CHANOPRIVSNEEDED
			sendMessage(socket, std::string(":localhost ") + ERR_CHANOPRIVSNEEDED + " " + client.nick + " " + channel->channelName + " :You're not channel operator\r\n");
			return;
		}
		// validar os modos
		
	}
	else // procurar nickname
	{
		Client* user = getClientByNick(message->arguments[0]);
		if (!user)
		{

		}
	}
}
