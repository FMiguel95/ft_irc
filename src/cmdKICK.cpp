#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.8
// Command: KICK
// Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
void Server::cmdKICK(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;
}
