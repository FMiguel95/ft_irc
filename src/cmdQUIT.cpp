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
		
		// if the user had an invite for that channel, remove it
		for (std::list<Client*>::iterator j = i->invitedUsers.begin(); j != i->invitedUsers.end(); ++j)
		{
			if ((*j)->nick == client.nick)
			{
				i->invitedUsers.erase(j);
				break;
			}
		}
		
		// if the channel is empty, remove it
		if (i->userList.empty())
		{
			channels.erase(i);
			break;
		}
	}
	//clients.erase(socket);
	client.isRegistered = false;
	client.nickOk = false;
	sendMessage(socket, std::string(":") + SERVER_NAME + " ERROR :Closing link\r\n");
}
