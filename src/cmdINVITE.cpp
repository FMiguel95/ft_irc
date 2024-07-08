#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.7
// Command: INVITE
// Parameters: <nickname> <channel>
void Server::cmdINVITE(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
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
