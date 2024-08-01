#include "../incs/Bot.hpp"
#include <iostream>
#include <cstring> // for memset
#include <unistd.h> // for close
#include <arpa/inet.h> // for inet_addr
#include <netdb.h> // for gethostbyname

Bot::Bot() {}

Bot::Bot(const std::string &serverAddress, int serverPort, const std::string &serverPassword) :
	_clientSocket(-1),
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
	_clientSocket(src._clientSocket),
	_serverAddress(src._serverAddress),
	_serverPort(src._serverPort),
	_serverPassword(src._serverPassword)
{}

Bot::~Bot() {}

Bot& Bot::operator =(const Bot& src)
{
	if (this != &src)
	{
		_clientSocket = src._clientSocket;
		_serverAddress = src._serverAddress;
		_serverPort = src._serverPort;
		_serverPassword = src._serverPassword;
	}
	return *this;
}

int Bot::sendMessage(const char *message)
{
	if (send(_clientSocket, message, strlen(message), 0) < 0)
	{
		std::cerr << RED << "Error sending message: " << RESET << strerror(errno) << std::endl;
		close(_clientSocket); // Close the socket before returning
		return -1;
	}
	std::cout << GREEN << "Message sent to the server: " << RESET << message;
	return 0;
}

int Bot::runBot()
{
	// Create a socket
	_clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_clientSocket < 0)
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
		close(_clientSocket); // Close the socket before returning
		return -1;
	}
	// Copy the server address to the sockaddr_in structure
	memcpy(&server_address.sin_addr, host->h_addr, host->h_length);

	// Connect to the server
	if (connect(_clientSocket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
		std::cerr << "Connection failed: " << strerror(errno) << std::endl;
		close(_clientSocket); // Close the socket before returning
		return -1;
	}

	std::cout << "Connected to the server!" << std::endl;

	// Send a message to the server
	std::string msg_pass = "PASS " + _serverPassword + "\r\n";
	std::string msg_nick = "NICK " + std::string(NICK) + "\r\n";
	std::string msg_user = "USER " + std::string(USER) + "\r\n";
	if (sendMessage(msg_pass.c_str()) < 0
		|| sendMessage(msg_nick.c_str()) < 0
		|| sendMessage(msg_user.c_str()) < 0)
		return -1;

	std::string msg_lisr = "JOIN #kindergarten\r\n";
	if (sendMessage(msg_lisr.c_str()) < 0)
		return -1;

	// Loop to receive messages from the server
	char buffer[1024];
	while (true)
	{
		memset(buffer, 0, sizeof(buffer)); // Clear buffer
		int bytes_received = recv(_clientSocket, buffer, sizeof(buffer), 0);
		if (bytes_received > 0)
		{
			std::cout << "bytes_received: " << bytes_received << std::endl;
			std::cout << "buffer: " << buffer << std::endl;
			std::string serverMessage = std::string(buffer, sizeof(buffer));
			receiveMessage(serverMessage);
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
	close(_clientSocket);
	return 0;
}

void Bot::receiveMessage(std::string& stream)
{
	_messageBuffer += stream;
	size_t pos;
	while ((pos = _messageBuffer.find_first_of("\r\n")) != std::string::npos)
	{
		std::string message = _messageBuffer.substr(0, pos);
		if (_messageBuffer[pos] == '\r' && pos + 1 < _messageBuffer.size() && _messageBuffer[pos + 1] == '\n')
			_messageBuffer.erase(0, pos + 2);
		else
			_messageBuffer.erase(0, pos + 1);
		std::cout << "\001\e[0;92m" << "Received: " << "\e[0m\002" << message << std::endl;
		if (parseMessage(message))
			handleMessage(&this->message);
	}
	//std::cout << "buffer:" << client.messageBuffer << std::endl;
}

t_message* Bot::parseMessage(std::string& stream)
{
	// split the message with " " as delimiter
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
			//message.prefix = *i;
			message.prefix = i->substr(1, i->length());
			continue;
		}
		// set command if first word doesnt start with : or if second word and prefix was already set
		if ((i == split.begin() && (*i)[0] != ':') || (i - 1 == split.begin() && !message.prefix.empty()))
		{
			message.command = *i;
			continue;
		}
		if ((*i)[0] == ':')
			*i = i->substr(1, i->length());
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

void Bot::handleMessage(t_message* message)
{
	if (!message)
	{
		//sendMessage("GARBAGE\r\n");
		return;
	}
	for (int i = 0; message->command[i]; i++)
		message->command[i] = std::toupper(message->command[i]);

	if (message->command == "PING")
		sendMessage("PONG\r\n");
}
