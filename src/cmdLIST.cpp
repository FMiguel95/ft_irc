#include "../inc/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.6
// Command: LIST
// Parameters: [ <channel> *( "," <channel> ) [ <target> ] ]
void Server::cmdLIST(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar se o user esta registado
	if (!client.isRegistered)
		return;
	
	if (message->arguments[0].empty()) // list every channel if no parameters
	{
		for (std::list<Channel>::iterator i = channels.begin(); i != channels.end(); ++i)
		{
			// reply RPL_LIST
			std::ostringstream oss;
			// if (i->topic.empty())
			// 	oss << ":") + SERVER_NAME + " " << RPL_LIST << " " << client.nick << " " << i->channelName << " " << i->userList.size() << "\r\n";
			// else
				oss << ":" << SERVER_NAME << " " << RPL_LIST << " " << client.nick << " " << i->channelName << " " << i->userList.size() << " :" << i->topic << "\r\n";
			sendMessage(socket, oss.str());
		}
	}
	else // else parse the parameter and only list those
	{
		std::vector<std::string> channelsSplit;

		// splitting the channels with "," as delimiter
		size_t start = 0;
		size_t end;
		while ((end = message->arguments[0].find(",", start)) != std::string::npos)
		{
			channelsSplit.push_back(message->arguments[0].substr(start, end - start));
			start = end + 1;
		}
		channelsSplit.push_back(message->arguments[0].substr(start));
		
		for (std::list<Channel>::iterator i = channels.begin(); i != channels.end(); ++i)
		{
			if (std::find(channelsSplit.begin(), channelsSplit.end(), i->channelName) != channelsSplit.end())
			{
				// reply RPL_LIST
				std::ostringstream oss;
				oss << ":" << SERVER_NAME << " " << RPL_LIST << " " << client.nick << " " << i->channelName << " " << i->userList.size() << " :" << i->topic << "\r\n";
				sendMessage(socket, oss.str());
			}
		}
	}
	// reply RPL_LISTEND
	sendMessage(socket, std::string(":") + SERVER_NAME " " + RPL_LISTEND + " " + client.nick + " :End of /LIST\r\n");
}
