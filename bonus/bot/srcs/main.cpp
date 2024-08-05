#include "../incs/Bot.hpp"
#include <csignal>


static void handle_sigint(int signal)
{
	if (signal == SIGINT || signal == SIGHUP || signal == SIGTERM)
		Bot::run = false;
}

int main(int ac, char** av)
{
	(void)av;
	if (ac != 4)
	{
		std::cerr << "Usage: ./dealer <address> <port> <password>" << std::endl;
		return 1;
	}

	std::signal(SIGINT, handle_sigint);
	std::signal(SIGHUP, handle_sigint);
	std::signal(SIGTERM, handle_sigint);
	Bot bot(av[1], std::atoi(av[2]), av[3]);
	return bot.runBot();
}
