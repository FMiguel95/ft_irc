#include "../incs/Bot.hpp"
#include <iostream>
#include <cstring> // for memset
#include <unistd.h> // for close
#include <arpa/inet.h> // for inet_addr
#include <netdb.h> // for gethostbyname

Bot::Bot() {}

Bot::Bot(const std::string &serverAddress, int serverPort, const std::string &serverPassword) : 
	_serverAddress(serverAddress),
	_serverPort(serverPort),
	_serverPassword(serverPassword)
{
	// Load the filtered words from a file
	std::ifstream file("filtered_words.txt");
	if (!file)
	{
		std::cerr << "Error opening file: " << strerror(errno) << std::endl;
	}
	else
	{
		std::string line;
		while (std::getline(file, line))
		{
			_filteredWords.push_back(line);
		}
		file.close();
	}
}

Bot::Bot(const Bot& src) :
	_serverAddress(src._serverAddress),
	_serverPort(src._serverPort),
	_serverPassword(src._serverPassword)
{}

Bot::~Bot() {}

Bot& Bot::operator =(const Bot& src)
{
	if (this != &src)
	{
		_serverAddress = src._serverAddress;
		_serverPort = src._serverPort;
		_serverPassword = src._serverPassword;
	}
	return *this;
}

int sendMessage(int client_socket, const char *message)
{
	if (send(client_socket, message, strlen(message), 0) < 0)
	{
		std::cerr << RED << "Error sending message: " << RESET << strerror(errno) << std::endl;
		close(client_socket); // Close the socket before returning
		return -1;
	}
	std::cout << GREEN << "Message sent to the server: " << RESET << message;
	return 0;
}

int Bot::runBot()
{
	// Create a socket
	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0)
	{
		std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
		return -1;
	}

	// Define server address
	sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address)); // Zero out the structure
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(_serverPort);

	// Resolve the server address
	struct hostent* host = gethostbyname(_serverAddress.c_str());
	if (host == NULL)
	{
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		close(client_socket); // Close the socket before returning
		return -1;
	}
	// Copy the server address to the sockaddr_in structure
	memcpy(&server_address.sin_addr, host->h_addr, host->h_length);

	// Connect to the server
	if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		std::cerr << "Connection failed: " << strerror(errno) << std::endl;
		close(client_socket); // Close the socket before returning
		return -1;
	}

	std::cout << "Connected to the server!" << std::endl;

	// Send a message to the server
	std::string msg_pass = "PASS " + _serverPassword + "\r\n";
	std::string msg_nick = "NICK " + std::string(NICK) + "\r\n";
	std::string msg_user = "USER " + std::string(USER) + "\r\n";
	if (sendMessage(client_socket, msg_pass.c_str()) < 0
		|| sendMessage(client_socket, msg_nick.c_str()) < 0
		|| sendMessage(client_socket, msg_user.c_str()) < 0)
		return -1;

	// Loop to receive messages from the server
	char buffer[1024];
	while (true)
	{
		memset(buffer, 0, sizeof(buffer)); // Clear buffer
		int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
		if (bytes_received > 0)
		{
			// a partir daqui o codigo e quase igual ao do servidor:
			// juntar mensagens num buffer estilo gnl ate aparecer \r\n
			// depois por cada linha completa, fazer parse e guardar o resultado num struct (?) ou algo do genero
			// depois decidir o que fazer dependendo do que foi recebido
		
			std::cout << YELLOW << buffer << RESET;
		}
		else if (bytes_received == 0)
		{
			std::cout << RED << "Server closed the connection" << RESET << std::endl;
			break;
		}
		else
		{
			std::cerr << RED << "Error receiving data: " << RESET << strerror(errno) << std::endl;
			break;
		}
	}
	// Close the socket
	close(client_socket);
	return 0;
}
