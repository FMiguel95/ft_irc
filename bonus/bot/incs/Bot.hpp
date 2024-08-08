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

#define YELLOW	"\033[33m"
#define RED		"\033[31m"
#define GREEN	"\033[32m"
#define RESET	"\033[0m"

#define NICK	"the_bot"
#define USER	"somebot 0 * :I'm a word filter bot!"

typedef struct
{
	std::string	raw;
	std::string	prefix;
	std::string	command;
	std::string	arguments[15];
} t_message;

class Bot
{
	public:
		static bool run;

		Bot(const std::string &serverAddress, int serverPort, const std::string &serverPassword);
		Bot(const Bot& src);
		~Bot();
		
		Bot& operator =(const Bot& src);

		int	runBot();

	private:
		int							_clientSocket;
		std::string					_serverAddress;
		int							_serverPort;
		std::string					_serverPassword;

		std::string					_messageBuffer;
		t_message					_message;
		std::vector<std::string>	_filteredWords;

		int			sendMessage(const std::string& message);
		void		receiveMessage(std::string& stream);
		t_message*	parseMessage(std::string& stream);
		void		handleMessage(t_message* message);
		void		scanMessage(t_message* message);
		
		Bot();
};
