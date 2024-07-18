#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.6.2
// Command: WHOIS
// Parameters: [ <target> ] <mask> *( "," <mask> )
void Server::cmdWHOIS(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);
	
	// validar se o user esta registado
	if (!client.isRegistered)
		return;
	
	// validar se tem parametros ERR_NEEDMOREPARAMS
	if (message->arguments[0].empty())
	{
		// reply ERR_NONICKNAMEGIVEN
		return;
	}

	int index = 0;

	// validar se o target existe
	Client *target = getClientByNick(message->arguments[0]);
	if (!message->arguments[1].empty())
	{
		// if the target is not a nick, it's a server -> reply ERR_NOSUCHSERVER
		if (target == NULL)
		{
			// not sure how it should behave look at this later
			// reply ERR_NOSUCHSERVER 
			return;
		}
		else
			target = getClientByNick(message->arguments[1]);
	}
	if (target == NULL)
	{
		// reply ERR_NOSUCHNICK
		return;
	
	}
	// if its successful 
	// RPL_WHOISUSER
	sendMessage(socket, std::string(":") + SERVER_NAME + " " + RPL_WHOISUSER + " " + client.nick + " " + target->nick + " " + target->user + " " + target->hostname + " * :" + target->realname + "\r\n");
	std::cout << target->realname << std::endl;
	// RPL_WHOISSERVER
	sendMessage(socket, std::string(":") + SERVER_NAME + " " + RPL_WHOISSERVER + " " + client.nick + " " + target->nick + " " + SERVER_NAME + " :A very cool server\r\n");
	// RPL_WHOISIDLE
	//sendMessage(socket, std::string(":") + SERVER_NAME + " " + RPL_WHOISIDLE + " " + client.nick + " " + target->nick + " " + target->getTimeSinceLastActivity() + " 0 :seconds idle, signon time\r\n");
	// RPL_WHOISCHANNELS
	// RPL_WHOISACTUALLY
	// RPL_AWAY - later
	// RPL_WHOISHOST
	sendMessage(socket, std::string(":") + SERVER_NAME + " " + RPL_WHOISHOST + " " + client.nick + " " + target->nick + " :is connecting from " + target->userAtHost + "\r\n");
	// reply RPL_ENDOFWHOIS
	sendMessage(socket, std::string(":") + SERVER_NAME + " " + RPL_ENDOFWHOIS + " " + client.nick + " " + target->nick + " :End of WHOIS list\r\n");
}

