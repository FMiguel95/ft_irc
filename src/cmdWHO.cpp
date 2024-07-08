#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.6.1
// Command: WHO
// Parameters: [ <mask> [ "o" ] ]
void Server::cmdWHO(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);
	
	if (client.passOk == false)
	{
		// reply ERR_PASSWDMISMATCH ??? see what to do 
		return;
	}

	// mask can be a nickname or a channel
	// if its a nickname then display info about that user
	// if its a channel then display info about all users in that channel

	// for each user reply RPL_WHOREPLY

	
	
	// reply RPL_ENDOFWHO

}
