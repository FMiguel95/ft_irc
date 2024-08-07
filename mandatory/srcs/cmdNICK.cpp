#include "../incs/Server.hpp"

static bool isNickValid(const std::string& nick)
{
	if (nick.length() > 63)
		return false;
	for (size_t i = 0; i < nick.size(); i++)
	{
		if (std::isalnum(nick[i]) == false && nick[i] != '\\' && nick[i] != '|' 
			&& nick[i] != '[' && nick[i] != ']' 
			&& nick[i] != '{' && nick[i] != '}'
			&& nick[i] != '_')
			return (false);
	}
	return (true);
}

static bool isNickinUse(const std::string& nick, std::map<int,Client>& clients)
{
	for (std::map<int,Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.isRegistered && it->second.nickOk && it->second.getUserInfo('n') == nick)
			return (true);
	}
	return (false);
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.2
// Command: NICK
// Parameters: <nickname>
void Server::cmdNICK(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);
	// este comando tem dois usos
	// - escolher o nick na altura do registo -> DONE
	// - mudar o nick depois de ja estar registado -> DONE

	// se ja estiver registado
	if (client.isRegistered)
	{
		// validar que o comando tem argumento
		if (message->arguments[0].empty())
		{
			// reply ERR_NONICKNAMEGIVEN
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NONICKNAMEGIVEN + " " + client.nick + " :No nickname given\r\n");
			return;
		}
		// validar caracteres do nick
		if (isNickValid(message->arguments[0]) == false)
		{
			// reply ERR_ERRONEUSNICKNAME
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_ERRONEUSNICKNAME + " " + client.nick + " " + message->arguments[0] + " :Erroneous nickname\r\n");
			return;
		}
		// validar que o nick nao esta a ser utilizado por outro user
		if (isNickinUse(message->arguments[0], _clients) == true)
		{
			// reply ERR_NICKNAMEINUSE
			sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NICKNAMEINUSE + " " + client.nick + " " + message->arguments[0] + " :Nickname is already in use\r\n");
			return;
		}
		// OK
		// ...notificar outros utilizadores da mudança?
		return;
	}

	// WHEN REGISTERING

	// validar que password é necessaria e enviou a pass correta
	//std::cout << "serverPass:" << serverPassword << " client.passOk" << client.passOk << std::endl;
	if (!_serverPassword.empty() && client.passOk == false)
	{
		std::cout << "ERR_PASSWDMISMATCH\n";
		// reply ERR_PASSWDMISMATCH ??? see what to do 
		//sendMessage(socket, std::string(":") + serverHostname + " " + ERR_PASSWDMISMATCH + " " + client.nick + " :Password incorrect\r\n");
		return;
	}
	// validar que o comando tem argumento -> the user can't be bigger than 9? copilot recommended this
	if (message->arguments[0].empty())
	{
		// reply ERR_NONICKNAMEGIVEN
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NONICKNAMEGIVEN + " :No nickname given\r\n");
		return;
	}
	// validar caracteres do nick
	if (isNickValid(message->arguments[0]) == false)
	{
		// reply ERR_ERRONEUSNICKNAME
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_ERRONEUSNICKNAME + " " + message->arguments[0] + " " + message->arguments[0] + " :Erroneous nickname\r\n");
		return;
	}
	// validar que o nick nao esta a ser utilizado por outro user
	if (isNickinUse(message->arguments[0], _clients) == true)
	{
		// reply ERR_NICKNAMEINUSE
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_NICKNAMEINUSE + " " + message->arguments[0] + " " + message->arguments[0] + " :Nickname is already in use\r\n");
		return;
	}
	// OK
	// add nick to the client
	client.nick = message->arguments[0];
	client.nickOk = true;

	checkRegistration(client);
}
