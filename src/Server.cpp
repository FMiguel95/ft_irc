#include "../inc/Server.hpp"

static int error_exit(const char *message)
{
	std::cerr << message << '\n';
	return 1;
}

Server::Server() : port(6667), password("") {}

Server::Server(const int& port, const std::string& password) : port(port), password(password) {}

Server::Server(const Server& src) :
port(src.port),
password(src.password),
clients(src.clients),
channels(src.channels)
{}

Server::~Server() {}

Server& Server::operator =(const Server& src)
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


	std::vector<pollfd>	fds;
	pollfd pfd;
	pfd.fd = serverSocket;
	pfd.events = POLLIN;
	pfd.revents = 0;
	fds.push_back(pfd);

	while (1)
	{
		int pollResult = poll(fds.data(), fds.size(), -1);
		if (pollResult == -1)
		{
			error_exit("Error polling sockets");
			break;
		}
		if (fds[0].revents & POLLIN)
		{
			sockaddr_in clientAddress;
			socklen_t		clientAddressSize = sizeof(clientAddress);
			int clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);
			//std::cout << clientSocket << std::endl;
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
				std::cout << "Received from client: " << clientMessage << "\n";

				receiveMessage(fds[i].fd, clientMessage);
			}
		}
	}
	return 0;
}

void Server::receiveMessage(const int& socket, std::string& stream)
{
	Client& client = clients.at(socket);
	client._messageBuffer += stream;
	std::cout << client._messageBuffer << std::endl;
	// TODO
	// check for \r\n
	// then remove from buffer and call parseMessage(std::string& stream);
}
