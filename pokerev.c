/*
 * pokerev.c
 * Copyright 2011 Fred Strauss
 *
 * Calculates Poker EV
 *
 * TODO Add GPL license here
 */

#include <stdio.h>

#include "poker_defs.h"
#include "inlines/eval.h"

void evalSingleTrial(StdDeck_CardMask player1, StdDeck_CardMask player2, StdDeck_CardMask board, 
	double wins[], int *numberOfTrials);

int main(int argc, char **argv) {

	// Say we start with something like this...
	int cardIndex = -1;

	// Player 1 has Ah Ad
	StdDeck_CardMask player1;
	StdDeck_stringToCard("Ah", &cardIndex);
	player1 = StdDeck_MASK(cardIndex);
	StdDeck_stringToCard("Ad", &cardIndex);
	StdDeck_CardMask_OR(player1, player1, StdDeck_MASK(cardIndex));

	// Player 2 has 8s9s
	StdDeck_CardMask player2;
	StdDeck_stringToCard("8s", &cardIndex);
	player2 = StdDeck_MASK(cardIndex);
	StdDeck_stringToCard("9s", &cardIndex);
	StdDeck_CardMask_OR(player2, player2, StdDeck_MASK(cardIndex));
	
	// Dead cards
	StdDeck_CardMask deadCards;
	StdDeck_CardMask_OR(deadCards, player1, player2);

	// Keep stats
	double wins[2] = { 0.0 };
	int numberOfTrials = 0;

	// Enumerate all possible 5 card boards
	StdDeck_CardMask boardCards;
	DECK_ENUMERATE_5_CARDS_D(StdDeck, boardCards, deadCards, 
		evalSingleTrial(player1, player2, boardCards, wins, &numberOfTrials); );

	// Convert wins to equity
	double p1Equity = (wins[0] / numberOfTrials) * 100.0;
	double p2Equity = (wins[1] / numberOfTrials) * 100.0;

	printf("Iterations: %d\r\n\r\n", numberOfTrials);
	printf("           Equity\r\n");
	printf("Player 1 : %0.4f %% : AhAd\r\n", p1Equity);
	printf("Player 2 : %0.4f %% : 8s9s\r\n", p2Equity);

	return 0;
}

void evalSingleTrial(StdDeck_CardMask player1, StdDeck_CardMask player2, StdDeck_CardMask board, 
	double wins[], int *numberOfTrials) {
	// Combine each player's hole cards with board cards
	StdDeck_CardMask_OR(player1, player1, board);
	StdDeck_CardMask_OR(player2, player2, board);

	// Evaluate each player's hand
	int p1Val = StdDeck_StdRules_EVAL_N(player1, 7);
	int p2Val = StdDeck_StdRules_EVAL_N(player2, 7);

	// keep track of wins
	if (p1Val > p2Val)
		wins[0] += 1.0;
	else if (p2Val > p1Val)
		wins[1] += 1.0;
	else {
		wins[0] += 0.5;
		wins[1] += 0.5;
	}

	(*numberOfTrials)++;
}
