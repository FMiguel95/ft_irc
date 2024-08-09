#include "../incs/Server.hpp"

bool	Server::run = false;

// Constructors, Destructor and Assignment operator -----------------------------------------------

Server::Server() :
		_serverPort(6667),
		_serverPassword(""),
		_serverCreationTime(std::time(NULL)),
		_messageOfTheDay("")
{
	getHostname();
	getMOTD();
}

Server::Server(const int& serverPort, const std::string& serverPassword) :
		_serverPort(serverPort),
		_serverPassword(serverPassword),
		_serverCreationTime(std::time(NULL)),
		_messageOfTheDay("")
{
	getHostname();
	getMOTD();
}

Server::Server(const Server& src) :
		_serverPort(src._serverPort),
		_serverPassword(src._serverPassword),
		_serverHostname(src._serverHostname),
		_serverCreationTime(src._serverCreationTime),
		_messageOfTheDay(src._messageOfTheDay),
		_clients(src._clients),
		_channels(src._channels) {}

Server::~Server() {}

Server& Server::operator = (const Server& src)
{
	if (this != &src)
	{
		_serverPort = src._serverPort;
		_serverPassword = src._serverPassword;
		_serverHostname = src._serverHostname;
		_clients = src._clients;
		_channels = src._channels;
		_serverCreationTime = src._serverCreationTime;
		_messageOfTheDay = src._messageOfTheDay;
	}
	return *this;
}

// Member Functions -------------------------------------------------------------------------------

static int error_exit(const char *message)
{
	std::cerr << message << '\n';
	return 1;
}

int Server::runServer()
{
	// CREATE A SOCKET ---------------------------------------------------------

	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
		return (error_exit("Error creating socket"));
	int opt = 1;
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return (error_exit("Error setting socket options"));

	// BIND SOCKET TO PORT -----------------------------------------------------

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(_serverPort);

	if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		return (error_exit("Error binding socket to port"));

	// LISTEN FOR CONNECTIONS --------------------------------------------------

	if (listen(serverSocket, SOMAXCONN) == -1)
		return (error_exit("Error listening for connections"));
	std::cout << "Server listening on port " << _serverPort << '\n';

	// POLLING -----------------------------------------------------------------

	std::vector<pollfd>	fds;
	pollfd pfd;

	pfd.fd = serverSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;
	fds.push_back(pfd);

	run = true;
	while (run)
	{
		int pollResult = poll(fds.data(), fds.size(), 100);
		if (pollResult == -1)
		{
			if (errno == EINTR)
				continue;
			error_exit("Error polling sockets");
			break;
		}
		// Accept new connections
		if (fds[0].revents & POLLIN)
		{
			sockaddr_in	clientAddress;
			socklen_t	clientAddressSize = sizeof(clientAddress);
			int			clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);
			
			if (clientSocket == -1)
			{
				error_exit("Error accepting connection");
				continue;
			}

			pfd.fd = clientSocket;
			pfd.events = POLLIN | POLLHUP | POLLERR;
			pfd.revents = 0;
			fds.push_back(pfd);

			Client client(clientSocket);
			client.hostname = inet_ntoa(clientAddress.sin_addr);
			_clients.insert(std::pair<int,Client>(clientSocket, client));
			std::cout << "\001\e[0;32m" << "New client connected: " << "\e[0m\002" << inet_ntoa(clientAddress.sin_addr) << ':' << ntohs(clientAddress.sin_port) << '\n';
		}
		// Check for incoming data from clients
		for (size_t i = 1; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				char buffer[1024];
				int bytesRead = recv(fds[i].fd, buffer, sizeof(buffer), 0);
				if (bytesRead <= 0)
				{
					std::cout << "\001\e[0;31m" << "Client " << fds[i].fd << " disconnected\n" << "\e[0m\002";
					Client& client = _clients.at(fds[i].fd);
					if (client.isRegistered)
						unregisterClient(client, "Client Quit");
					_clients.erase(fds[i].fd);
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					i--;
					continue;
				}
				std::string clientMessage = std::string(buffer, 0, bytesRead);
				receiveMessage(fds[i].fd, clientMessage);
			}
			if ((fds[i].revents & POLLHUP) || (fds[i].revents & POLLERR))
			{
				std::cerr << "\001\e[0;31m" << "Client " << fds[i].fd << " poll error\n" << "\e[0m\002";
				_clients.erase(fds[i].fd);
				close(fds[i].fd);
				fds.erase(fds.begin() + i);
				i--;
			}
		}
		checkTimeouts(fds);
	}
	for (std::vector<pollfd>::iterator i = fds.begin(); i != fds.end(); ++i)
		close(i->fd);
	return (0);
}

void Server::sendMessage(const int& socket, const std::string& message)
{
	// Sends a message to a client
	std::cout << "\001\e[0;93m" << "Sending to socket " << socket << ": " << "\e[0m\002" << message;
	send(socket, message.c_str(), message.size(), 0);
}

void Server::broadcastMessage(Channel& channel, const std::string& message)
{
	// Broadcasts a message to all users in a channel
	for (std::map<Client*,char>::iterator i = channel.userList.begin(); i != channel.userList.end(); ++i)
		sendMessage(i->first->socket, message);
}

void Server::receiveMessage(const int& socket, std::string& stream)
{
	// Append the received message to the buffer
	Client& client = _clients.at(socket);
	client.updateActivityTime();
	client.messageBuffer += stream;
	size_t pos;
	while ((pos = client.messageBuffer.find_first_of("\r\n")) != std::string::npos)
	{
		std::string message = client.messageBuffer.substr(0, pos);
		if (client.messageBuffer[pos] == '\r' && pos + 1 < client.messageBuffer.size() && client.messageBuffer[pos + 1] == '\n')
			client.messageBuffer.erase(0, pos + 2);
		else
			client.messageBuffer.erase(0, pos + 1);
		std::cout << "\001\e[0;92m" << "Received from socket " << socket << ": " << "\e[0m\002" << message << std::endl;
		if (parseMessage(message))
			handleMessage(socket, &this->_message);
	}
}

t_message* Server::parseMessage(std::string& stream)
{
	// Split the message using spaces as delimiters
	std::vector<std::string> split;
	size_t start = stream.find_first_not_of(" ", 0);
	size_t end;
	while (start != std::string::npos && (end = stream.find(" ", start)) != std::string::npos && split.size() < 15 && !(stream[start] == ':' && split.size() > 1))
	{
		split.push_back(stream.substr(start, end - start));
		start = stream.find_first_not_of(" ", end + 1);
	}
	if (start != std::string::npos)
		split.push_back(stream.substr(start));

	// Fill the message structure with the prefix, command and arguments
	_message.raw = stream;
	_message.prefix.clear();
	_message.command.clear();
	for (size_t i = 0; i < 15; i++)
		_message.arguments[i].clear();

	int argIndex = 0;
	for (std::vector<std::string>::iterator i = split.begin(); i != split.end(); ++i)
	{
		// Set prefix if first word starts with :
		if (i == split.begin() && (*i)[0] == ':')
		{
			_message.prefix = i->substr(1, i->length());
			continue;
		}
		// Set command if first word doesnt start with : or if second word and prefix was already set
		if ((i == split.begin() && (*i)[0] != ':') || (i - 1 == split.begin() && !_message.prefix.empty()))
		{
			_message.command = *i;
			continue;
		}
		// Set arguments
		if ((*i)[0] == ':')
			*i = i->substr(1, i->length());
		_message.arguments[argIndex++] = *i;
	}
	if (_message.command.empty())
		return NULL;
	return &_message;
}

void Server::handleMessage(const int& socket, t_message* message)
{
	// Handle the message received from the server and respond accordingly
	if (!message)
	{
		sendMessage(socket, "GARBAGE\r\n");
		return;
	}
	for (int i = 0; message->command[i]; i++)
		message->command[i] = std::toupper(message->command[i]);

	if (message->command == "PASS")
		cmdPASS(socket, message);
	else if (message->command == "NICK")
		cmdNICK(socket, message);
	else if (message->command == "USER")
		cmdUSER(socket, message);
	else if (message->command == "JOIN")
		cmdJOIN(socket, message);
	else if (message->command == "PART")
		cmdPART(socket, message);
	else if (message->command == "PRIVMSG")
		cmdPRIVMSG(socket, message);
	else if (message->command == "NOTICE")
		cmdNOTICE(socket, message);
	else if (message->command == "MODE")
		cmdMODE(socket, message);
	else if (message->command == "TOPIC")
		cmdTOPIC(socket, message);
	else if (message->command == "KICK")
		cmdKICK(socket, message);
	else if (message->command == "INVITE")
		cmdINVITE(socket, message);
	else if (message->command == "LIST")
		cmdLIST(socket, message);
	else if (message->command == "WHO")
		cmdWHO(socket, message);
	else if (message->command == "WHOIS")
		cmdWHOIS(socket, message);
	else if (message->command == "QUIT")
		cmdQUIT(socket, message);
	else if (message->command == "PING")
		cmdPING(socket, message);
	else if (message->command == "PONG")
		cmdPONG(socket, message);
	else if (message->command == "CAP")
		return;
	else
		sendMessage(socket, std::string(":") + _serverHostname + " " + ERR_UNKNOWNCOMMAND + " " + _clients.at(socket).nick + " " + message->command + " :Unknown command\r\n");
}

bool Server::isChannelNameValid(const std::string& name) const
{
	// Checks if the channel name is valid
	if (name[0] != '#' && name[0] != '&')
		return false;
	for (size_t i = 1; i < name.size(); i++)
	{
		if (name[i] == ' ' || name[i] == ',' || name[i] == 7)
			return false;
	}
	return true;
}

void Server::checkRegistration(Client& client)
{
	// Checks if the client has sent valid pass, nick and user commands and sends the welcome message
	if (!client.isRegistered && (_serverPassword.empty() || client.passOk) && client.nickOk && client.userOk)
	{
		client.isRegistered = true;
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_WELCOME + " " + client.nick + " :Welcome to the " + SERVER_NAME + " Internet Relay Network, " + client.nick + "!\r\n");
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_YOURHOST + " " + client.nick + " :Your host is " + _serverHostname + ", running version v0.1\r\n");
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_CREATED + " " + client.nick + " :This server was created " + std::asctime(std::localtime(&_serverCreationTime)));
		sendMessage(client.socket, std::string(":") + _serverHostname + " " + RPL_MYINFO + " " + client.nick + " " + _serverHostname + " v0.1 o iklt\r\n");
		sendMOTD(client);
	}
}

void Server::checkTimeouts(std::vector<pollfd>& fds)
{
	// Check for timeouts and ping clients if necessary
	for (size_t i = 1; i < fds.size(); i++)
	{
		Client& client = _clients.at(fds[i].fd);
		double diff = client.getTimeSinceLastActivity();
		if (client.isRegistered && diff > TIMEOUT_TIME && client.pendingPong == false)
		{
			// Ping the client
			sendMessage(client.socket, std::string(":") + _serverHostname + " PING\r\n");
			client.updatePingTime();
			client.pendingPong = true;
		}
		else if (!client.isRegistered && diff > TIMEOUT_TIME)
		{
			// Disconnect the client
			std::cout << "client " << client.socket << ": disconnect unregistered" << std::endl;
			_clients.erase(fds[i].fd);
			close(fds[i].fd);
			fds.erase(fds.begin() + i);
			i--;
			continue;
		}
		diff = client.getTimeSinceLastPing();
		if (client.isRegistered && client.pendingPong && diff > PING_TIMEOUT_TIME)
		{
			// Disconnect the client
			std::ostringstream reason;
			reason << "Ping timeout: " << diff << " seconds";
			unregisterClient(client, reason.str());
			sendMessage(client.socket, std::string(":") + _serverHostname + " ERROR :" + reason.str() + "\r\n");
			_clients.erase(fds[i].fd);
			close(fds[i].fd);
			fds.erase(fds.begin() + i);
			i--;
		}
	}
}

void Server::unregisterClient(Client& client, const std::string& reason)
{
	// Makes sure the client is gone from the server
	for (std::list<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
	{
		std::map<Client*,char>::iterator userInChannel = i->getClientInChannel(client.nick);
		if (userInChannel == i->userList.end())
			continue;
		i->userList.erase(userInChannel);
		
		// Notify other clients that the user has quit
		broadcastMessage(*i, std::string(":") + client.nick + "!" + client.userAtHost + " QUIT :" + reason + "\r\n");
		
		// If the user had an invite for that channel, remove it
		for (std::list<Client*>::iterator j = i->invitedUsers.begin(); j != i->invitedUsers.end(); ++j)
		{
			if ((*j)->nick == client.nick)
			{
				i->invitedUsers.erase(j);
				break;
			}
		}
		
		// If the channel is now empty, remove it from the server
		if (i->userList.empty())
		{
			_channels.erase(i);
			break;
		}
	}
	client.isRegistered = false;
	client.nickOk = false;
}

void Server::getMOTD()
{
	// Open input file
	std::ifstream motdFile("MOTD.txt");
	if (!motdFile)
	{
		std::cerr << "Error opening MOTD.txt" << std::endl;
		_hasMOTD = false;
		return;
	}
	_hasMOTD = true;

	// Read file line by line
	for (std::string line; std::getline(motdFile, line); )
	{
		_messageOfTheDay.append(line);
		if (!motdFile.eof())
			_messageOfTheDay.push_back('\n');
	}
	motdFile.close();
}

void Server::getHostname()
{
	// Gets the hostname from /etc/hostname
	std::ifstream hostnameFile("/etc/hostname");
	if (!hostnameFile)
	{
		std::cerr << "Error opening /etc/hostname" << std::endl;
		_serverHostname = "localhost";
		return;
	}

	std::string hostname;
	std::getline(hostnameFile, hostname);
	_serverHostname = hostname;
}

Channel* Server::getChannelByName(const std::string& name)
{
	// Returns a channel if there is one with the given name
	for (std::list<Channel>::iterator i = _channels.begin(); i != _channels.end(); ++i)
	{
		if (i->channelName == name)
			return (Channel*)&*i;
	}
	return NULL;
}

Client* Server::getClientByNick(const std::string& nick)
{
	// Returns a client if there is one with the given nick
	for (std::map<int, Client>::iterator i = _clients.begin(); i != _clients.end(); ++i)
	{
		if (i->second.nick == nick && i->second.isRegistered)
			return (Client*)&i->second;
	}
	return NULL;
}
