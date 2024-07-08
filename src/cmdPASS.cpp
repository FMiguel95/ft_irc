#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.1
// Command: PASS
// Parameters: <password>
void Server::cmdPASS(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);
	
	// se ja estiver registado
	if (client.isRegistered)
	{
		// reply ERR_ALREADYREGISTRED
		return; 
	}
	// se o servidor nao precisar de password OK
	if (password.empty())
	{
		client.passOk = true;
		return;
	}
	// se faltar argumento
	if (message->arguments[0].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		return; 
	}
	// se a pass esta mal
	if (password != message->arguments[0])
	{
		// reply ERR_PASSWDMISMATCH
		return; 
	}
	// se a pass der match
	if (password == message->arguments[0])
	{
		client.passOk = true;
		return; 
	}
	// add password to the class?
}
