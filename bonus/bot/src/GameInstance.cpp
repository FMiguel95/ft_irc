#include "../inc/GameInstance.hpp"

GameInstance::GameInstance() {}

GameInstance::GameInstance(const GameInstance& src) {}

GameInstance::~GameInstance() {}

GameInstance& GameInstance::operator =(const GameInstance& src)
{
	if (this != &src)
	{
		_deck = src._deck;
		_cardsPlayer = src._cardsPlayer;
		_cardsDealer = src._cardsDealer;
	}
	return *this;
}
