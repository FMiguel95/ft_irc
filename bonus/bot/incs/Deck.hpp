#pragma once

#include <deque>
#include <cstdlib>

typedef enum {
	spades,
	clubs,
	hearts,
	diamonds
} suit_e;

typedef enum {
	ace = 1,
	two,
	three,
	four,
	five,
	six,
	seven,
	eight,
	nine,
	ten,
	jack,
	queen,
	king
} rank_e;

typedef struct {
	suit_e suit;
	rank_e rank;
} card_t;

class Deck
{
public:
	Deck();
	Deck(const Deck& src);
	~Deck();
	Deck& operator =(const Deck& src);

	void shuffleDeck();
	card_t* takeCard();
	void addCard(card_t& card);

private:
	std::deque<card_t> _deck;
};
