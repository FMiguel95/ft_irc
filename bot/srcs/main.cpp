#include "../incs/Bot.hpp"
#include <csignal>


static void handle_sigint(int signal)
{
	if (signal == SIGINT || signal == SIGHUP || signal == SIGTERM)
		Bot::run = false;
}

static bool str2uint16(char* str, int* out)
{
	*out = 0;
	int i = 0;
	if (str == NULL || str[0] == '\0')
		return false;
	while (str[i])
	{
		if (!std::isdigit(str[i]))
			return false;
		*out = *out * 10 + str[i] - '0';
		if (*out > 0xFFFF)
			return false;
		i++;
	}
	return true;
}

int main(int ac, char** av)
{
	if (ac != 4)
	{
		std::cerr << "Usage: ./dealer <address> <port> <password>" << std::endl;
		return 1;
	}

	std::string address = av[1];
	int port;
	if (!str2uint16(av[2], &port))
	{
		std::cerr << "Invalid port" << std::endl;
		return 1;
	}
	std::string password = av[3];

	std::signal(SIGINT, handle_sigint);
	std::signal(SIGHUP, handle_sigint);
	std::signal(SIGTERM, handle_sigint);
	
	Bot bot(address, port, password);
	return bot.runBot();
}
