#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.3
// Command: USER
// Parameters: <user> <mode> <unused> <realname>
void Server::cmdUSER(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);
	
	// validar que ainda nao fez registo
	if (client.isRegistered)
	{
		// reply ERR_ALREADYREGISTRED
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_ALREADYREGISTRED + " " + client.nick + " :You may not reregister\r\n");
		return;
	}

	// validar que password é necessaria e enviou a pass correta
	if (!serverPassword.empty() && client.passOk == false)
	{
		// reply ERR_PASSWDMISMATCH ??? see what to do
		//sendMessage(socket, std::string(":") + serverHostname + " " + ERR_PASSWDMISMATCH + " " + client.nick + " :Password incorrect\r\n"); 
		return;
	}

	// validar que tem os argumentos todos -> ERR_NEEDMOREPARAMS -> DONE
	if (message->arguments[0].empty() || message->arguments[1].empty() 
		|| message->arguments[2].empty() || message->arguments[3].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		sendMessage(socket, std::string(":") + serverHostname + " " + ERR_NEEDMOREPARAMS + " " + client.nick + " USER :Not enough parameters\r\n");
		return;
	}

	// add user to the client and log the client in
	client.user = std::string("~") + message->arguments[0];
	client.userAtHost = client.user + "@" + client.hostname;
	client.realname = message->arguments[3];
	client.userOk = true;

	checkRegistration(client);
}
