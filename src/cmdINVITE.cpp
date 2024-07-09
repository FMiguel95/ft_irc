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
	std::map<int, Client>::iterator it1;
	for (it1 = clients.begin(); it1 != clients.end(); ++it1)
	{
		if (it1->second.nick == message->arguments[0])
			break;
	}
	if (it1 == clients.end())
	{
		// reply ERR_NOSUCHNICK
		return;
	}

	// se o canal existir, validar que o user esta no canal reply ERR_NOTONCHANNEL
	std::list<Channel>::iterator it2;
	for (it2 = channels.begin(); it2 != channels.end(); ++it2)
	{
		if (it2->channelName.compare(message->arguments[1]) == 0)
		{
			// validar que o nick esta no canal
			std::map<Client *, char>::iterator it3;
			for (it3 = it2->userList.begin(); it3 != it2->userList.end(); ++it3)
			{
				if (it3->first->nick == client.nick)
					break;
			}
			if (it3 == it2->userList.end())
			{
				// reply ERR_NOTONCHANNEL
				return;
			}
			// se o canal for invite only, validar que o user é operator reply ERR_CHANOPRIVSNEEDED
			if (it2->channelMode & MODE_i)
			{
				if (!(it3->second & MODE_o))
				{
					// reply ERR_CHANOPRIVSNEEDED
					return;
				}
			}
			// validar que o recipient nao esta no canal reply ERR_USERONCHANNEL
			std::map<Client *, char>::iterator it4;
			for (it4 = it2->userList.begin(); it4 != it2->userList.end(); ++it4)
			{
				if (it4->first->nick == message->arguments[0])
				{
					// reply ERR_USERONCHANNEL
					return;
				}
			}
			break;
		}
	}
	// se OK reply RPL_INVITING

	// adicionar o user à lista de convidados
	it2->invitedUsers.push_back(&it1->second);
}
