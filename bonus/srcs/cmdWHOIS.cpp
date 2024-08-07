#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.6.2
// Command: WHOIS
// Parameters: [ <target> ] <mask> *( "," <mask> )
void Server::cmdWHOIS(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);
	
	// validar se o user esta registado
	if (!client.isRegistered)
		return;
	
	// validar se tem parametros ERR_NEEDMOREPARAMS
	if (message->arguments[0].empty())
	{
		// reply ERR_NONICKNAMEGIVEN
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NONICKNAMEGIVEN + " " + client.nick + " :No nickname given\r\n");
		return;
	}

	// validar se o target existe
	Client *target = getClientByNick(message->arguments[0]);
	if (!message->arguments[1].empty())
	{
		// if the target is not a nick, it's a server -> reply ERR_NOSUCHSERVER
		if (target == NULL)
		{
			// not sure how it should behave look at this later
			// reply ERR_NOSUCHSERVER 
			//sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NOSUCHSERVER + " " + client.nick + " " + message->arguments[1] + " :No such server\r\n");
			return;
		}
		else
			target = getClientByNick(message->arguments[1]);
	}
	if (target == NULL)
	{
		// reply ERR_NOSUCHNICK
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NOSUCHNICK + " " + client.nick + " " + message->arguments[0] + " :No such nick/channel\r\n");
		return;
	
	}

	// if its successful:

	// RPL_WHOISUSER
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOISUSER + " " + client.nick + " " + target->nick + " " + target->user + " " + target->hostname + " * :" + target->realname + "\r\n");
	
	// RPL_WHOISSERVER
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOISSERVER + " " + client.nick + " " + target->nick + " " + _serverHostname + " :A very cool server\r\n");
	
	// RPL_WHOISHOST
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOISHOST + " " + client.nick + " " + target->nick + " :is connecting from " + target->userAtHost + "\r\n");
	
	// RPL_WHOISCHANNELS: will show the channels the user is in
	std::string channelList;
	for (std::list<Channel>::iterator it1 = _channels.begin(); it1 != _channels.end(); ++it1)
	{
		for (std::map<Client*,char>::iterator it2 = it1->userList.begin(); it2 != it1->userList.end(); ++it2)
		{
			if (it2->first == target)
				channelList += it1->channelName + " ";
		}
	}
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_WHOISCHANNELS + " " + client.nick + " " + target->nick + " :" + channelList + "\r\n");

	// reply RPL_ENDOFWHOIS
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_ENDOFWHOIS + " " + client.nick + " " + target->nick + " :End of WHOIS list\r\n");
	
	// RPL_WHOISIDLE -LATER
	//sendMessage(socket, std::string(":") + serverHostname + " " + RPL_WHOISIDLE + " " + client.nick + " " + target->nick + " " + target->getTimeSinceLastActivity() + " 0 :seconds idle, signon time\r\n");
	// RPL_AWAY - LATER
}

