#include "../incs/Bot.hpp"

int main(int ac, char** av)
{
	if (ac != 4)
	{
		std::cerr << "Usage: ./dealer <address> <port> <password>" << std::endl;
		return 1;
	}

	Bot bot;
	return bot.runBot();
}
