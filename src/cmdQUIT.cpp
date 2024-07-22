#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.7
// Command: QUIT
// Parameters: [ <Quit Message> ]
void Server::cmdQUIT(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;

	sendMessage(socket, std::string(":") + client.nick + "!" + client.userAtHost + " QUIT :Client Quit\r\n");

	for (std::list<Channel>::iterator i = channels.begin(); i != channels.end(); ++i)
	{
		std::map<Client*,char>::iterator userInChannel = i->getClientInChannel(client.nick);
		if (userInChannel == i->userList.end())
			continue;
		i->userList.erase(userInChannel);
		// notificar outros users da saida
		broadcastMessage(*i, std::string(":") + client.nick + "!" + client.userAtHost + " QUIT :Client Quit\r\n");
	}
	//clients.erase(socket);
	client.isRegistered = false;
	client.nickOk = false;
	sendMessage(socket, std::string(":") + SERVER_NAME + " ERROR :Closing link\r\n");
}
