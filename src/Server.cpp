#include "../inc/Server.hpp"

static int error_exit(const char *message)
{
	std::cerr << message << '\n';
	return 1;
}

Server::Server() : port(6667), password("") {}

Server::Server(const int& port, const std::string& password) : 
											port(port), password(password) {}

Server::Server(const Server& src) :
							port(src.port),
							password(src.password),
							clients(src.clients),
							channels(src.channels) {}

Server::~Server() {}

Server& Server::operator = (const Server& src)
{
	if (this != &src)
	{
		port = src.port;
		password = src.password;
		clients = src.clients;
		channels = src.channels;
	}
	return *this;
}

int Server::run()
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
	serverAddress.sin_port = htons(port);

	if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		return (error_exit("Error binding socket to port"));

	// LISTEN FOR CONNECTIONS --------------------------------------------------

	if (listen(serverSocket, SOMAXCONN) == -1)
		return (error_exit("Error listening for connections"));
	std::cout << "Server listening on port " << port << '\n';

	// POLLING -----------------------------------------------------------------

	std::vector<pollfd>	fds;
	pollfd pfd;

	pfd.fd = serverSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;
	fds.push_back(pfd);

	while (1)
	{
		int pollResult = poll(fds.data(), fds.size(), 100);
		if (pollResult == -1)
		{
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

			Client client;
			clients.insert(std::pair<int,Client>(clientSocket, client));
			std::cout << "New client connected: " << inet_ntoa(clientAddress.sin_addr) << ':' << ntohs(clientAddress.sin_port) << '\n';
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
					if (bytesRead == 0)
						std::cout << "Client disconnected\n";
					else
					{
						std::cout << bytesRead << std::endl;
						error_exit("Error receiving data from client");
					}
					close(fds[i].fd);
					fds.erase(fds.begin() + i);
					clients.erase(fds[i].fd);
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

void Server::receiveMessage(const int& socket, std::string& stream)
{
	Client& client = clients.at(socket);
	client.updateActivityTime();
	client._messageBuffer += stream;
	while (client._messageBuffer.find("\r\n") != std::string::npos)
	{
		std::string message = client._messageBuffer.substr(0, client._messageBuffer.find("\r\n"));
		client._messageBuffer.erase(0, client._messageBuffer.find("\r\n") + 2);
		std::cout << "Received from Client: " << message << std::endl;
		if (parseMessage(message))
			handleMessage(socket, &this->message);
	}
	// version coppied from above harcodeded to be used with nc
	while (client._messageBuffer.find("\n") != std::string::npos)
	{
		std::string message = client._messageBuffer.substr(0, client._messageBuffer.find("\n"));
		client._messageBuffer.erase(0, client._messageBuffer.find("\n") + 1);
		std::cout << "Received from Client: " << message << std::endl;
		if (parseMessage(message))
			handleMessage(socket, &this->message);
	}
	std::cout << "buffer:" << client._messageBuffer << std::endl;

}

t_message* Server::parseMessage(std::string& stream)
{
	// split the message with " " as delimiter
	std::vector<std::string> split;
	std::string token;
	size_t start = 0, end;
	while ((end = stream.find(" ", start)) != std::string::npos)
	{
		split.push_back(stream.substr(start, end - start));
		start = end + 1;
	}
	split.push_back(stream.substr(start));

	// initialize struct
	message.prefix.clear();
	message.command.clear();
	for (size_t i = 0; i < 15; i++)
		message.arguments[i].clear();
	int j = 0;
	for (std::vector<std::string>::iterator i = split.begin(); i != split.end(); ++i)
	{
		// std::cout << *i << std::endl;
		if (i == split.begin())
		{
			message.command = *i;
			continue;
		}
		message.arguments[j++] = *i;
	}
	std::cout << "struct prefix:" << message.prefix << std::endl;
	std::cout << "struct command:" << message.command << std::endl;
	for (size_t i = 0; i < 15; i++)
		std::cout << "struct arg " << i << ":" << message.arguments[i] << std::endl;

	return &message;
}

void Server::handleMessage(const int& socket, t_message* message)
{
	if (message->command == "PASS")
		cmdPASS(socket, message);
	else if (message->command == "NICK")
		cmdNICK(socket, message);
	else if (message->command == "USER")
		cmdUSER(socket, message);
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.1
// Command: PASS
// Parameters: <password>
void Server::cmdPASS(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);
	
	// se ja estiver registado
	if (client.isRegistered)
	{
		// reply ERR_ALREADYREGISTRED
		return; 
	}
	// se o servidor nao precisar de password OK
	if (password.empty())
	{
		client.passOk = true;
		return;
	}
	// se faltar argumento
	if (message->arguments[0].empty())
	{
		// reply ERR_NEEDMOREPARAMS
		return; 
	}
	// se a pass esta mal
	if (password != message->arguments[0])
	{
		// reply ERR_PASSWDMISMATCH
		return; 
	}
	// se a pass der match
	if (password == message->arguments[0])
	{
		client.passOk = true;
		return; 
	}
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.2
// Command: NICK
// Parameters: <nickname>
void Server::cmdNICK(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// este comando tem dois usos
	// - escolher o nick na altura do registo
	// - mudar o nick depois de ja estar registado
	// se for o segundo caso, notificar outros utilizadores da mudança?


	// se nao estiver registado
	// 
	// validar que ja enviou a pass correta
	// validar que o comando tem argumento -> ERR_NONICKNAMEGIVEN
	// validar caracteres do nick -> ERR_ERRONEUSNICKNAME
	// validar que o nick nao esta a ser utilizado por outro user -> ERR_NICKNAMEINUSE

	// se ja estiver registado
	// 
	// validar que o comando tem argumento -> ERR_NONICKNAMEGIVEN
	// validar caracteres do nick -> ERR_ERRONEUSNICKNAME
	// validar que o nick nao esta a ser utilizado por outro user -> ERR_NICKNAMEINUSE
	// OK
	// ...notificar outros utilizadores da mudança?
}

// https://datatracker.ietf.org/doc/html/rfc2812#section-3.1.3
// Command: USER
// Parameters: <user> <mode> <unused> <realname>
void Server::cmdUSER(const int& socket, const t_message* message)
{
	Client& client = clients.at(socket);

	// validar que ja enviou a pass correta
	// validar que ainda nao fez registo -> ERR_ALREADYREGISTRED
	// validar que tem os argumentos todos -> ERR_NEEDMOREPARAMS





	// se a pass nick e user do client estiverem OK permitir login no servidor
}
