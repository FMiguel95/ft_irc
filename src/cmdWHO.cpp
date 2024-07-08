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
}
