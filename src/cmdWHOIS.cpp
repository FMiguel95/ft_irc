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
	if (!message->arguments[1].empty())
	{
		// if the target is not a nick, it's a server -> reply ERR_NOSUCHSERVER
		if (getClientByNick(message->arguments[0]) == NULL)
		{
			// not sure how it should behave look at this later
			// reply ERR_NOSUCHSERVER 
			return;
		}
		else
			index = 1;
	}

	// validate that the nick (<mask>) exists -> reply ERR_NOSUCHNICK
	if (getClientByNick(message->arguments[index]) == NULL)
	{
		// reply ERR_NOSUCHNICK
		return;
	}

	// if its successful reply RPL_WHOISUSER, RPL_WHOISSERVER, RPL_ENDOFWHOIS
	// reply ERR_ENDOFWHOIS
}
