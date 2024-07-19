#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.1
// Command: PASS
// Parameters: <password>
void Server::cmdPASS(const int& socket, const t_message* message)
{
	std::cout << "clients.size(): " << clients.size() << std::endl;
	Client& client = clients.at(socket);
	
	// se ja estiver registado
	if (client.isRegistered)
	{
		// reply ERR_ALREADYREGISTRED
		sendMessage(socket, std::string(":") + SERVER_NAME + " " + ERR_ALREADYREGISTRED + " " + client.nick + " :You may not reregister\r\n");
		return; 
	}
	// se o servidor nao precisar de password OK
	if (serverPassword.empty())
	{
		client.passOk = true;
		return;
	}
	// se faltar argumento
	if (message->arguments[0].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		sendMessage(socket, std::string(":") + SERVER_NAME + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " PASS :Not enough parameters\r\n");
		return; 
	}
	// se a pass esta mal
	if (serverPassword != message->arguments[0])
	{
		// reply ERR_PASSWDMISMATCH
		sendMessage(socket, std::string(":") + SERVER_NAME + " " + ERR_PASSWDMISMATCH + " " + client.nick + " :Password incorrect\r\n");
		return; 
	}
	// se a pass der match
	if (serverPassword == message->arguments[0])
	{
		client.passOk = true;
		return; 
	}
	// add password to the class?
}

