#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.3.1
// Command: PRIVMSG
// Parameters: <msgtarget> <text to be sent>
void Server::cmdPRIVMSG(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
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

	// para enviar a mensagem, é so fazer relay do message->arguments[1] para cada recipient

}
