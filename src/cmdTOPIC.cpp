#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.4
// Command: TOPIC
// Parameters: <channel> [ <topic> ]
void Server::cmdTOPIC(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
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
	// se não, validar que o canal tem modo -t reply RPL_TOPIC ou RPL_NOTOPIC
	// validar que é operator reply RPL_TOPIC ou RPL_NOTOPIC
	// se não, reply ERR_CHANOPRIVSNEEDED
}
