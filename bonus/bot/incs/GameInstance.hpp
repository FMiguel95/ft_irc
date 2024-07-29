#pragma once

#include "Deck.hpp"
#include <vector>

class GameInstance
{
public:
	GameInstance();
	GameInstance(const GameInstance& src);
	~GameInstance();
	GameInstance& operator =(const GameInstance& src);

	int calculatePoints(std::vector<card_t>& cards);
	void playerHit();	// the player chose to ask for another card
	void playerStand();	// the player chose to not ask for another card
	// void playerSplit(); // ???

private:
	Deck _deck;
	std::vector<card_t> _cardsPlayer;
	std::vector<card_t> _cardsDealer;
};
