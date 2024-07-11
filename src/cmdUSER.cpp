#include "../inc/Server.hpp"

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
		return;
	}
	// validar que password é necessaria e enviou a pass correta
	if (!serverPassword.empty() && client.passOk == false)
	{
		// reply ERR_PASSWDMISMATCH ??? see what to do 
		return;
	}
	// validar que tem os argumentos todos -> ERR_NEEDMOREPARAMS -> DONE
	if (message->arguments[0].empty() || message->arguments[1].empty() 
		|| message->arguments[2].empty() || message->arguments[3].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		return;
	}
	// add user to the client and log the client in
	client.user = std::string("~") + message->arguments[0];// + "@" + client.hostname;
	client.userAtHost = client.user + "@" + client.hostname;
	client.userOk = true;

	// se a pass nick e user do client estiverem OK permitir login no servidor
	if ((serverPassword.empty() || client.passOk) && client.nickOk && client.userOk)
	{
		client.isRegistered = true;
		sendMessage(socket, std::string(":localhost ") + RPL_WELCOME + " " + client.nick + " :Welcome to the Internet Relay Network, " + client.nick + "!\r\n");
		sendMessage(socket, std::string(":localhost ") + RPL_YOURHOST + " " + client.nick + " :Your host is localhost, running version v0.1\r\n");
		sendMessage(socket, std::string(":localhost ") + RPL_CREATED + " " + client.nick + " :This server was created yesterday\r\n");
		sendMessage(socket, std::string(":localhost ") + RPL_MYINFO + " " + client.nick + " localhost v0.1 o itkl\r\n"); // ?
	}
}
