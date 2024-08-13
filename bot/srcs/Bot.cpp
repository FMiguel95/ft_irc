#include "../incs/Bot.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

bool	Bot::run = false;

// Constructors, Destructor and Assignment operator -----------------------------------------------

Bot::Bot() {}

Bot::Bot(const std::string &serverAddress, int serverPort, const std::string &serverPassword) :
		_clientSocket(-1),
		_serverAddress(serverAddress),
		_serverPort(serverPort),
		_serverPassword(serverPassword)
{
	// Load words to be filtered from file
	std::ifstream file("filtered_words.txt");
	if (!file)
	{
		std::cerr << "Error opening file" << std::endl;
	}
	else
	{
		std::string line;
		while (std::getline(file, line))
		{
			if (line.empty())
				continue;
			_filteredWords.push_back(line);
		}
		file.close();
	}
}

Bot::Bot(const Bot& src) :
		_clientSocket(src._clientSocket),
		_serverAddress(src._serverAddress),
		_serverPort(src._serverPort),
		_serverPassword(src._serverPassword) {}

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

// Member functions -------------------------------------------------------------------------------

int Bot::runBot()
{
	// CREATE A SOCKET ---------------------------------------------------------
	
	_clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_clientSocket < 0)
	{
		std::cerr << "Error creating socket" << std::endl;
		return -1;
	}

	// Set socket option - timeout
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(_clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

	// DEFINE SERVER ADDRESS ---------------------------------------------------
	
	sockaddr_in server_address;
	std::memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(_serverPort);

	// Check if the server address is valid
	struct hostent* host = gethostbyname(_serverAddress.c_str());
	if (host == NULL)
	{
		std::cerr << "Invalid address/ Address not supported" << std::endl;
		close(_clientSocket);
		return -1;
	}
	std::memcpy(&server_address.sin_addr, host->h_addr, host->h_length);

	// CONNECT TO THE SERVER ---------------------------------------------------

	if (connect(_clientSocket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
	{
		std::cerr << "Connection failed" << std::endl;
		close(_clientSocket);
		return -1;
	}
	std::cout << "Connected to the server!" << std::endl;

	// Send the necessary messages to the server to enter the server and join a channel
	sendMessage("PASS " + _serverPassword + "\r\n");
	sendMessage("NICK " + std::string(NICK) + "\r\n");
	sendMessage("USER " + std::string(USER) + "\r\n");
	sendMessage("JOIN #kindergarten\r\n");

	// CHECK FOR MESSAGES FROM THE SERVER --------------------------------------

	run = true;
	while (run)
	{
		std::cout << "test\n";
		char buffer[1024];
		std::memset(buffer, '\0', 1024);
		int bytes_received = recv(_clientSocket, buffer, sizeof(buffer), 0);
		if (bytes_received > 0)
		{
			std::string serverMessage;
			if (bytes_received == 1024 && buffer[1023] != '\0')
			{
				char* newBuffer = new char[1025];
				std::memcpy(newBuffer, buffer, 1024);
				newBuffer[1024] = '\0';
				serverMessage = std::string(buffer, bytes_received);
				delete[](newBuffer);
			}
			else
			{
				buffer[bytes_received] = '\0';
				serverMessage = std::string(buffer, bytes_received);
			}
			receiveMessage(serverMessage);
		}
		else if (bytes_received == 0)
		{
			std::cout << RED << "Server closed the connection" << RESET << std::endl;
			break;
		}
		else
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;
			std::cerr << RED << "Error receiving data" << RESET << std::endl;
			break;
		}
	}
	close(_clientSocket);
	return 0;
}

int Bot::sendMessage(const std::string& message)
{
	// Send the message to the server
	if (send(_clientSocket, message.c_str(), message.length(), 0) < 0)
	{
		std::cerr << RED << "Error sending message" << RESET << std::endl;
		close(_clientSocket);
		return -1;
	}
	std::cout << GREEN << "Message sent to the server: " << RESET << message;
	return 0;
}

void Bot::receiveMessage(std::string& stream)
{
	// Append the received message to the buffer
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
			handleMessage(&this->_message);
	}
}

t_message* Bot::parseMessage(std::string& stream)
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

void Bot::handleMessage(t_message* message)
{
	// Handle the message received from the server and respond accordingly
	if (!message)
		return;
	for (int i = 0; message->command[i]; i++)
		message->command[i] = std::toupper(message->command[i]);

	if (message->command == "PING")
		sendMessage("PONG\r\n");
	else if (message->command == "INVITE")
	{
		sendMessage(std::string("JOIN ") + message->arguments[1] + "\r\n");
		sendMessage("PRIVMSG " + message->arguments[1] + " :Hello! I'm a word filter bot! Give me operator status and I will kick users who use bad words!\r\n");
	}
	else if (message->command == "PRIVMSG")
		scanMessage(message);
}

void Bot::scanMessage(t_message* message)
{
	// Check if the message contains any forbidden words. If it does, kick the user from the channel
	if (!message)
		return;
	for (int i = 0; message->arguments[i].length(); i++)
	{
		for (std::vector<std::string>::iterator it = _filteredWords.begin(); it != _filteredWords.end(); ++it)
		{
			if (message->arguments[i].find(*it) != std::string::npos)
			{
				std::string nick = message->prefix.substr(0, message->prefix.find("!"));
				sendMessage("KICK " + message->arguments[0] + " " + nick + " :Word " + *it + " is not allowed here!\r\n");
				return;
			}
		}
	}
}
