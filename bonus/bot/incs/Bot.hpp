#pragma once

#include "GameInstance.hpp"
#include <iostream>
#include <map>

class Bot
{
public:
	Bot();
	Bot(const Bot& src);
	~Bot();
	Bot& operator =(const Bot& src);

	int runBot();

private:
	std::map<std::string,GameInstance> _games; // list of games with client nickname as key
};
