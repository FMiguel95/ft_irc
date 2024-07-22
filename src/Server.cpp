#include "../inc/Server.hpp"

static int error_exit(const char *message)
{
	std::cerr << message << '\n';
	return 1;
}

bool Server::run = false;

Server::Server() :
serverPort(6667),
serverPassword(""),
serverCreationTime(std::time(NULL)),
messageOfTheDay("")
{
	// open input file
	std::ifstream infile("MOTD");
	if (!infile)
	{
		std::cerr << "MOTD: " << std::strerror(errno) << std::endl;
		hasMOTD = false;
		return;
	}
	// read from input file
	hasMOTD = true;
	for (std::string line; std::getline(infile, line); )
	{
		messageOfTheDay.append(line);
		if (!infile.eof())
			messageOfTheDay.push_back('\n');
	}
	infile.close();
}

Server::Server(const int& serverPort, const std::string& serverPassword) :
serverPort(serverPort),
serverPassword(serverPassword),
serverCreationTime(std::time(NULL)),
messageOfTheDay("")
{
	// open input file
	std::ifstream infile("MOTD");
	if (!infile)
	{
		std::cerr << "MOTD: " << std::strerror(errno) << std::endl;
		hasMOTD = false;
		return;
	}
	// read from input file
	hasMOTD = true;
	for (std::string line; std::getline(infile, line); )
	{
		messageOfTheDay.append(line);
		if (!infile.eof())
			messageOfTheDay.push_back('\n');
	}
	infile.close();
}

Server::Server(const Server& src) :
							serverPort(src.serverPort),
							serverPassword(src.serverPassword),
							clients(src.clients),
							channels(src.channels),
							serverCreationTime(src.serverCreationTime),
							messageOfTheDay(src.messageOfTheDay)
							{}

Server::~Server() {}

Server& Server::operator = (const Server& src)
{
	if (this != &src)
	{
		serverPort = src.serverPort;
		serverPassword = src.serverPassword;
		clients = src.clients;
		channels = src.channels;
		serverCreationTime = src.serverCreationTime;
		messageOfTheDay = src.messageOfTheDay;
	}
	return *this;
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
	serverAddress.sin_port = htons(serverPort);

	if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		return (error_exit("Error binding socket to port"));

	// LISTEN FOR CONNECTIONS --------------------------------------------------

	if (listen(serverSocket, SOMAXCONN) == -1)
		return (error_exit("Error listening for connections"));
	std::cout << "Server listening on port " << serverPort << '\n';

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
		// ACCEPT CONNECTION FROM CLIENT
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
			//std::cout << "hostname: " << client.hostname << std::endl;
			clients.insert(std::pair<int,Client>(clientSocket, client));
			std::cout << "\001\e[0;32m" << "New client connected: " << "\e[0m\002" << inet_ntoa(clientAddress.sin_addr) << ':' << ntohs(clientAddress.sin_port) << '\n';
		}
		// SEE IF THERE IS DATA TO READ FROM CLIENT
		for (size_t i = 1; i < fds.size(); i++)
		{
			if (fds[i].revents & POLLIN)
			{
				char buffer[1024];
				int bytesRead = recv(fds[i].fd, buffer, sizeof(buffer), 0);
				if (bytesRead <= 0)
				{
					std::cout << "\001\e[0;31m" << "Client " << fds[i].fd << " disconnected\n" << "\e[0m\002";
					// else
					// 	error_exit("Error receiving data from client");
					clients.erase(fds[i].fd);
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					i--;
					continue;
				}
				std::string clientMessage = std::string(buffer, 0, bytesRead);
				receiveMessage(fds[i].fd, clientMessage);
			}
		}
	}
	return (0);
}

void Server::sendMessage(const int& socket, const std::string& message)
{
	std::cout << "\001\e[0;93m" << "Sending to socket " << socket << ": " << "\e[0m\002" << message;
	send(socket, message.c_str(), message.size(), 0);
}

void Server::broadcastMessage(Channel& channel, const std::string& message)
{
	for (std::map<Client*,char>::iterator i = channel.userList.begin(); i != channel.userList.end(); ++i)
	{
		sendMessage(i->first->socket, message);
	}
}

void Server::receiveMessage(const int& socket, std::string& stream)
{
	Client& client = clients.at(socket);
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
			handleMessage(socket, &this->message);
	}
	// // version coppied from above harcodeded to be used with nc
	// while (client.messageBuffer.find("\n") != std::string::npos)
	// {
	// 	std::string message = client.messageBuffer.substr(0, client.messageBuffer.find("\n"));
	// 	client.messageBuffer.erase(0, client.messageBuffer.find("\n") + 1);
	// 	std::cout << "\001\e[0;92m" << "Received from socket " << socket << ": " << "\e[0m\002" << message << std::endl;
	// 	if (parseMessage(message))
	// 		handleMessage(socket, &this->message);
	// }
	//std::cout << "buffer:" << client.messageBuffer << std::endl;
}

t_message* Server::parseMessage(std::string& stream)
{
	// split the message with " " as delimiter
	std::vector<std::string> split;
	size_t start = 0;
	size_t end;
	while ((end = stream.find(" ", start)) != std::string::npos && split.size() < 15 && stream[start] != ':')
	{
		split.push_back(stream.substr(start, end - start));
		start = end + 1;
	}
	if (stream[start] == ':')	// remove extra ':'
		start++;
	split.push_back(stream.substr(start));

	// initialize struct
	message.raw = stream;
	message.prefix.clear();
	message.command.clear();
	for (size_t i = 0; i < 15; i++)
		message.arguments[i].clear();
	int argIndex = 0;
	for (std::vector<std::string>::iterator i = split.begin(); i != split.end(); ++i)
	{
		// std::cout << *i << std::endl;
		// set prefix if first word starts with :
		if (i == split.begin() && (*i)[0] == ':')
		{
			message.prefix = *i;
			continue;
		}
		// set command if first word doesnt start with : or if second word and prefix was already set
		if ((i == split.begin() && (*i)[0] != ':') || (i - 1 == split.begin() && !message.prefix.empty()))
		{
			message.command = *i;
			continue;
		}
		message.arguments[argIndex++] = *i;
	}
	// std::cout << "struct prefix:" << message.prefix << std::endl;
	// std::cout << "struct command:" << message.command << std::endl;
	// for (size_t i = 0; i < 15; i++)
	// 	std::cout << "struct arg " << i << ":" << message.arguments[i] << std::endl;

	if (message.command.empty())
		return NULL;
	return &message;
}

void Server::handleMessage(const int& socket, t_message* message)
{
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
	else // reply ERR_UNKNOWNCOMMAND
		sendMessage(socket, std::string(":") + SERVER_NAME " " + ERR_UNKNOWNCOMMAND + " " + clients.at(socket).nick + " " + message->command + " :Unknown command\r\n");
}

Channel* Server::getChannelByName(const std::string& name)
{
	for (std::list<Channel>::iterator i = channels.begin(); i != channels.end(); ++i)
	{
		if (i->channelName == name)
			return (Channel*)&*i;
	}
	return NULL;
}

Client* Server::getClientByNick(const std::string& nick)
{
	for (std::map<int, Client>::iterator i = clients.begin(); i != clients.end(); ++i)
	{
		if (i->second.nick == nick && i->second.isRegistered)
			return (Client*)&i->second;
	}
	return NULL;
}

bool Server::isChannelNameValid(const std::string& name) const
{
	//std::cout << "name: " << name << std::endl;	
	if (name[0] != '#' && name[0] != '&')
	{
		// std::cout << name[0] << " invalid >:(" << std::endl;
		return false;
	}
	for (size_t i = 1; i < name.size(); i++)
	{
		if (name[i] == ' ' || name[i] == ',' || name[i] == 7)
		{
			// std::cout << "invalid!! >:(" << std::endl;
			return false;
		}
	}
	// std::cout << "valid" << std::endl;
	return true;
}

void Server::checkRegistration(Client& client)
{
	// se a pass nick e user do client estiverem OK permitir login no servidor
	if (!client.isRegistered && (serverPassword.empty() || client.passOk) && client.nickOk && client.userOk)
	{
		client.isRegistered = true;
		sendMessage(client.socket, std::string(":") + SERVER_NAME " " + RPL_WELCOME + " " + client.nick + " :Welcome to the Internet Relay Network, " + client.nick + "!\r\n");
		sendMessage(client.socket, std::string(":") + SERVER_NAME " " + RPL_YOURHOST + " " + client.nick + " :Your host is " + SERVER_NAME + ", running version v0.1\r\n");
		sendMessage(client.socket, std::string(":") + SERVER_NAME " " + RPL_CREATED + " " + client.nick + " :This server was created " + std::asctime(std::localtime(&serverCreationTime)));
		sendMessage(client.socket, std::string(":") + SERVER_NAME " " + RPL_MYINFO + " " + client.nick + " localhost v0.1 o iklt\r\n"); // ?
		// sendMessage(socket, std::string(":") + SERVER_NAME " " + RPL_ISUPPORT + " " + client.nick + " ? ?? ??? :are supported by this server\r\n"); // ?
		sendMOTD(client);
	}
}
