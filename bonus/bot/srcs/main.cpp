#include "../incs/Bot.hpp"

int main(int ac, char** av)
{
	(void)av;
	if (ac != 4)
	{
		std::cerr << "Usage: ./dealer <address> <port> <password>" << std::endl;
		return 1;
	}

	Bot bot(av[1], std::atoi(av[2]), av[3]);
	return bot.runBot();
}
