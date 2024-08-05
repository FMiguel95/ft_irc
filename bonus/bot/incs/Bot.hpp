#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cctype>
#include <ctime>
#include <sstream>
#include <cerrno>
#include <cstring>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define YELLOW "\033[33m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

#define NICK "the_bot"
#define USER "somebot 0 * :I'm a word filter bot!"

typedef struct {
	std::string raw;
	std::string prefix;
	std::string command;
	std::string arguments[15];
} t_message;

class Bot
{
public:
	static bool run;

	Bot(const std::string &serverAddress, int serverPort, const std::string &serverPassword);
	Bot(const Bot& src);
	~Bot();
	Bot& operator =(const Bot& src);

	int runBot();

private:
	int _clientSocket;
	std::string	_serverAddress;
	int			_serverPort;
	std::string	_serverPassword;

	std::string _messageBuffer;
	t_message message;
	std::vector<std::string> _filteredWords;

	int sendMessage(const char *message);

	void receiveMessage(std::string& stream);

	// faz parse à string
	// retira o prefix, command e args e passa para um struct
	// retorna nulo se for invalido
	t_message* parseMessage(std::string& stream);

	// decide o que fazer com a mensagem resultante
	void handleMessage(t_message* message);
	
	Bot();
};
