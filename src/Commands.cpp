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
	client.nick = message->arguments[0];
	client.nickOk = true;
	// add nick to the client

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

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.3
// Command: USER
// Parameters: <user> <mode> <unused> <realname>
void Server::cmdUSER(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);
	
	if (client.passOk == false)
	{
		// reply ERR_PASSWDMISMATCH ??? see what to do 
		return;
	}
	if (client.getUserInfo('u').empty() == false)
	{
		// reply ERR_ALREADYREGISTRED
		return;
	}
	if (message->arguments[0].empty() || message->arguments[1].empty() 
		|| message->arguments[2].empty() || message->arguments[3].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		return;
	}
	client.user = message->arguments[0];
	client.userOk = true;
	// add user to the client and log the client in

	// validar que ja enviou a pass correta -> DONE
	// validar que ainda nao fez registo -> ERR_ALREADYREGISTRED -> DONE
	// validar que tem os argumentos todos -> ERR_NEEDMOREPARAMS -> DONE

	// se a pass nick e user do client estiverem OK permitir login no servidor
	if ((password.empty() || client.passOk) && client.nickOk && client.userOk)
	{
		client.isRegistered = true;
		sendMessage(socket, std::string(":localhost") + " 001 " + client.nick + " :Welcome to the Internet Relay Network, " + client.nick + "!\r\n");
		sendMessage(socket, std::string(":localhost") + " 002 " + client.nick + " :Your host is localhost, running version v0.1\r\n");
		sendMessage(socket, std::string(":localhost") + " 003 " + client.nick + " :This sever was created yesterday\r\n");
		sendMessage(socket, std::string(":localhost") + " 004 " + client.nick + " localhost v0.1 iowghraAs biklmnopstve\r\n"); // ?
	}
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.1
// Command: JOIN
// Parameters: ( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0"
void Server::cmdJOIN(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta logado
	if (!client.isRegistered)
		return;
	
	if (message->arguments[0].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		return;
	}

	std::vector<std::string> channels;
	std::vector<std::string> keys;

	// splitting the channels with "," as delimiter
	size_t start = 0;
	size_t end;
	while ((end = message->arguments[0].find(",", start)) != std::string::npos)
	{
		channels.push_back(message->arguments[0].substr(start, end - start));
		start = end + 1;
	}
	channels.push_back(message->arguments[0].substr(start));

	// splitting the keys with "," as delimiter
	start = 0;
	while ((end = message->arguments[1].find(",", start)) != std::string::npos)
	{
		keys.push_back(message->arguments[1].substr(start, end - start));
		start = end + 1;
	}
	keys.push_back(message->arguments[1].substr(start));

	// for (size<<<_t i = 0; i < channels.size(); i++)
	// 	std::cout << channels[i] << std::endl;
	// for (size_t i = 0; i < keys.size(); i++)
	// 	std::c>>>out << keys[i] << std::endl;

	for (std::vector<std::string>::iterator i; i != channels.end(); ++i)
	{
		// validar se o nome nao contem caracteres incorretos
		// verificar se o canal ja existe
			// validar se o canal tem user limit e ja esta cheio
			// validar se é invite only
			// validar se esta protegido por password e a key é correta
		// se nao existir, criar e adicionar o user como operator

		// if the JOIN is successful the server will reply with:
		// a JOIN message
		// the channels topic with RPL_TOPIC, or RPL_NOTOPIC if no topic is set
		// the list of users currently in the channel

		// notify the other users in the channel

	}


	// validacoes de invites etcccccccccccccc
	
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.3.1
// Command: PRIVMSG
// Parameters: <msgtarget> <text to be sent>
void Server::cmdPRIVMSG(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta logado
	if (!client.isRegistered)
		return;

	// validar se tem target
	if (message->arguments[0].empty())
	{
		// reply ERR_NORECIPIENT
		return;
	}
	// validar se tem conteudo
	if (message->arguments[1].empty())
	{
		// reply ERR_NOTEXTTOSEND
		return;
	}

	// se enviar para um canal
	// se nao existir reply ERR_CANNOTSENDTOCHAN
	// se o user estiver banido, não fazer nada


	// se enviar para outro user
	// se o nick nao existir reply ERR_NOSUCHNICK


	// para enviar a mensagem, é so fazer relay do const t_message* message para cada recipient
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.3
// Command: MODE
// Parameters: <channel> *( ( "-" / "+" ) *<modes> *<modeparams> )
void Server::cmdMODE(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta logado
	if (!client.isRegistered)
		return;
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.4
// Command: TOPIC
// Parameters: <channel> [ <topic> ]
void Server::cmdTOPIC(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta logado
	if (!client.isRegistered)
		return;

	// validar parametros
	if (message->arguments[0].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		return;
	}

	// validar que o user está no canal reply ERR_NOTONCHANNEL

	// se nao tiver segundo argumento apenas enviar o topico do canal
	if (message->arguments[1].empty())
	{
		// reply RPL_TOPIC ou RPL_NOTOPIC se não existir
		return;
	}

	// se for para alterar o topico do canal
	// validar que é operator reply RPL_TOPIC ou RPL_NOTOPIC
	// se não, validar que o canal tem modo -t reply RPL_TOPIC ou RPL_NOTOPIC
	// se não, reply ERR_CHANOPRIVSNEEDED
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.8
// Command: KICK
// Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
void Server::cmdKICK(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta logado
	if (!client.isRegistered)
		return;
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.7
// Command: INVITE
// Parameters: <nickname> <channel>
void Server::cmdINVITE(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta logado
	if (!client.isRegistered)
		return;

	// validar parametros
	if (message->arguments[0].empty() || message->arguments[1].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		return;
	}

	// validar que o nick existe reply ERR_NOSUCHNICK
	// validar que o esta no canal reply ERR_NOTONCHANNEL
	// validar que o user é operator reply ERR_CHANOPRIVSNEEDED
	// validar que o recipient nao esta no canal reply ERR_USERONCHANNEL

	// se OK reply RPL_INVITING

}
