#include "../inc/Server.hpp"

static bool isNickValid(const std::string& nick)
{
	for (size_t i = 0; i < nick.size(); i++)
	{
		if (std::isalnum(nick[i]) == false && nick[i] != '\\' && nick[i] != '|' 
			&& nick[i] != '[' && nick[i] != ']' 
			&& nick[i] != '{' && nick[i] != '}')
			return (false);
	}
	return (true);
}

static bool isNickinUse(const std::string& nick, std::map<int,Client>& clients)
{
	for (std::map<int,Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (it->second.isRegistered && it->second.getUserInfo('n') == nick)
			return (true);
	}
	return (false);
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.2
// Command: NICK
// Parameters: <nickname>
void Server::cmdNICK(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);
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
			return;
		}
		// validar caracteres do nick
		if (isNickValid(message->arguments[0]) == false)
		{
			// reply ERR_ERRONEUSNICKNAME
			return;
		}
		// validar que o nick nao esta a ser utilizado por outro user
		if (isNickinUse(message->arguments[0], clients) == true)
		{
			// reply ERR_NICKNAMEINUSE
			return;
		}
		// OK
		// ...notificar outros utilizadores da mudança?
		return;
	}

	// WHEN REGISTERING

	// validar que password é necessaria e enviou a pass correta
	//std::cout << "serverPass:" << serverPassword << " client.passOk" << client.passOk << std::endl;
	if (!serverPassword.empty() && client.passOk == false)
	{
		std::cout << "ERR_PASSWDMISMATCH\n";
		// reply ERR_PASSWDMISMATCH ??? see what to do 
		return;
	}
	// validar que o comando tem argumento -> the user can't be bigger than 9? copilot recommended this
	if (message->arguments[0].empty())
	{
		// reply ERR_NONICKNAMEGIVEN
		return;
	}
	// validar caracteres do nick
	if (isNickValid(message->arguments[0]) == false)
	{
		// reply ERR_ERRONEUSNICKNAME
		return;
	}
	// validar que o nick nao esta a ser utilizado por outro user
	if (isNickinUse(message->arguments[0], clients) == true)
	{
		// reply ERR_NICKNAMEINUSE
		return;
	}
	// OK
	// add nick to the client
	client.nick = message->arguments[0];
	client.nickOk = true;
}
