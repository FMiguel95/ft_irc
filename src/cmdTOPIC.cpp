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

	// se canal tiver modo +t e user nao for operator reply ERR_CHANOPRIVSNEEDED

	// tudo OK, reply RPL_TOPIC ou RPL_NOTOPIC para todos os users no canal???
}
