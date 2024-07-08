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
		if (it->second.getUserInfo('n') == nick)
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

	// WHEN ADDING THE NICK
	if (client.getUserInfo('n').empty() == true)
	{
		if (client.passOk == false)
		{
			// reply ERR_PASSWDMISMATCH ??? see what to do 
			return;
		}
	}
	// WHEN CHANGING THE NICK
	if (message->arguments[0].empty())
	{
		// reply ERR_NONICKNAMEGIVEN
		return;
	}
	if (isNickValid(message->arguments[0]) == false)
	{
		// reply ERR_ERRONEUSNICKNAME
		return;
	}
	if (isNickinUse(message->arguments[0], clients) == true)
	{
		// reply ERR_NICKNAMEINUSE
		return;
	}
	// add nick to the client
	client.nick = message->arguments[0];
	client.nickOk = true;

	// este comando tem dois usos
	// - escolher o nick na altura do registo -> DONE
	// - mudar o nick depois de ja estar registado -> DONE
	// se for o segundo caso, notificar outros utilizadores da mudança?

	// se nao estiver registado
	// 
	// validar que ja enviou a pass correta -> did this is cmdPASS  -> DONE
	// validar que o comando tem argumento -> ERR_NONICKNAMEGIVEN -> the user can't be bigger than 9? copilot recommended this
	// validar caracteres do nick -> ERR_ERRONEUSNICKNAME -> DONE
	// validar que o nick nao esta a ser utilizado por outro user -> ERR_NICKNAMEINUSE -> DONE

	// se ja estiver registado
	// 
	// validar que o comando tem argumento -> ERR_NONICKNAMEGIVEN -> DONE
	// validar caracteres do nick -> ERR_ERRONEUSNICKNAME  -> DONE
	// validar que o nick nao esta a ser utilizado por outro user -> ERR_NICKNAMEINUSE -> DONE
	// OK
	// ...notificar outros utilizadores da mudança?
}
