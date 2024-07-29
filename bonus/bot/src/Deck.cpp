#include "../inc/Deck.hpp"

Deck::Deck()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 1; i <= 13; j++)
		{
			card_t card;
			card.suit = (suit_e)i;
			card.rank = (rank_e)j;
			_deck.push_back(card);
		}
	}
}

Deck::Deck(const Deck& src) : _deck(src._deck) {}

Deck::~Deck() {}

Deck& Deck::operator =(const Deck& src)
{
	if (this != &src)
	{
		_deck = src._deck;
	}
	return *this;
}
