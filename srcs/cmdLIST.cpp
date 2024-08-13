#include "../incs/Server.hpp"

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.2.6
// Command: LIST
// Parameters: [ <channel> *( "," <channel> ) [ <target> ] ]
void Server::cmdLIST(const int& socket, const t_message* message)
{
	Client& client = _clients.at(socket);

	// Validate that the client is registered
	if (!client.isRegistered)
		return;
	
	// If there are no parameters, list all channels
	if (message->arguments[0].empty())
	{
		for (std::list<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
		{
			std::ostringstream oss;
			oss << ":" << _serverHostname << " " << RPL_LIST << " " << client.nick << " " << i->channelName << " " << i->userList.size() << " :" << i->topic << "\r\n";
			sendMessage(socket, oss.str());
		}
	}
	// If there are parameters, list the channels specified
	else
	{
		// Split the channels with "," as delimiter
		std::vector<std::string> channelsSplit;
		size_t start = 0;
		size_t end;
		while ((end = message->arguments[0].find(",", start)) != std::string::npos)
		{
			channelsSplit.push_back(message->arguments[0].substr(start, end - start));
			start = end + 1;
		}
		channelsSplit.push_back(message->arguments[0].substr(start));
		
		for (std::list<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
		{
			if (std::find(channelsSplit.begin(), channelsSplit.end(), i->channelName) != channelsSplit.end())
			{
				std::ostringstream oss;
				oss << ":" << _serverHostname << " " << RPL_LIST << " " << client.nick << " " << i->channelName << " " << i->userList.size() << " :" << i->topic << "\r\n";
				sendMessage(socket, oss.str());
			}
		}
	}
	sendMessage(socket, std::string(":") + _serverHostname + " " + RPL_LISTEND + " " + client.nick + " :End of /LIST\r\n");
}
