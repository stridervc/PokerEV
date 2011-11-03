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

int main(int argc, char **argv) {

	// Say we start with something like this...
	int cardIndex = -1;

	StdDeck_CardMask player1;
	StdDeck_stringToCard("9h", &cardIndex);
	player1 = StdDeck_MASK(cardIndex);
	StdDeck_stringToCard("9d", &cardIndex);
	StdDeck_CardMask_OR(player1, player1, StdDeck_MASK(cardIndex));

	StdDeck_CardMask player2;
	StdDeck_stringToCard("Kh", &cardIndex);
	player2 = StdDeck_MASK(cardIndex);
	StdDeck_stringToCard("Kd", &cardIndex);
	StdDeck_CardMask_OR(player2, player2, StdDeck_MASK(cardIndex));
	
	// // Get each player's full 7-card hand into his mask
	//StdDeck_CardMask_OR(player1, player1, theBoard);
	//StdDeck_CardMask_OR(player2, player2, theBoard);
	
	// // Evaluate each player's hand
	int player1Val = StdDeck_StdRules_EVAL_N(player1, 7);
	int player2Val = StdDeck_StdRules_EVAL_N(player2, 7);
	
	if (player1Val > player2Val)
	   printf("Player 1 wins!\r\n");
	else if (player1Val < player2Val)
	   printf("Player 2 wins!\r\n");
	else
	   printf("It's a tie!\r\n");
	
	return 0;
}
