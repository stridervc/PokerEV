/*
 * pokerev.c
 * Copyright 2011 Fred Strauss
 *
 * Calculates Poker EV
 *
 * TODO Add GPL license here
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "poker_defs.h"
#include "inlines/eval.h"

void 	evalSingleTrial(StdDeck_CardMask player1, StdDeck_CardMask player2, StdDeck_CardMask board, 
			double wins[], double ties[], int *numberOfTrials);
StdDeck_CardMask	txtToMask(const char *txt);
void	cleanInput(char *hand);

int main(int argc, char **argv) {

	// Read two hands from keyboard
	char 				hand1str[10];
	char				hand2str[10];
	StdDeck_CardMask	hand1, hand2;

	printf("Hand 1 : ");
	scanf("%s", hand1str);
	cleanInput(hand1str);
	hand1 = txtToMask(hand1str);

	printf("Hand 2 : ");
	scanf("%s", hand2str);
	cleanInput(hand2str);
	hand2 = txtToMask(hand2str);

	int cardIndex = -1;

	// Dead cards
	StdDeck_CardMask deadCards;
	StdDeck_CardMask_OR(deadCards, hand1, hand2);

	// Keep stats
	double wins[2] = { 0.0 };
	double ties[2] = { 0.0 };
	int numberOfTrials = 0;

	// Enumerate all possible 5 card boards
	StdDeck_CardMask boardCards;
	DECK_ENUMERATE_5_CARDS_D(StdDeck, boardCards, deadCards, 
		evalSingleTrial(hand1, hand2, boardCards, wins, ties, &numberOfTrials); );

	// Convert wins to equity
	double h1Equity = ((wins[0] + ties[0]/2) / numberOfTrials) * 100.0;
	double h2Equity = ((wins[1] + ties[1]/2) / numberOfTrials) * 100.0;
	double h1Wins = (wins[0] / numberOfTrials) * 100.0;
	double h2Wins = (wins[1] / numberOfTrials) * 100.0;
	double h1Ties = (ties[0] / numberOfTrials) * 100.0;
	double h2Ties = (ties[1] / numberOfTrials) * 100.0;

	printf("\r\n");
	printf("         Equity    : Win       : Tie \r\n");
	printf("Hand 1 : %0.4f %% : %0.4f %% : %0.4f %% : %s\r\n", h1Equity, h1Wins, h1Ties, hand1str);
	printf("Hand 2 : %0.4f %% : %0.4f %% : %0.4f %% : %s\r\n", h2Equity, h2Wins, h2Ties, hand2str);

	return 0;
}

void evalSingleTrial(StdDeck_CardMask player1, StdDeck_CardMask player2, StdDeck_CardMask board, 
	double wins[], double ties[], int *numberOfTrials) {
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
		ties[0] += 1.0;
		ties[1] += 1.0;
	}

	(*numberOfTrials)++;
}


// Converts plaintext to cardmask
StdDeck_CardMask	txtToMask(const char *txt) {
	StdDeck_CardMask	hand;		// the hand to return
	StdDeck_CardMask	currCard;	// current card mask
	int					index;		// tmp card index valie
	char				card[3];
	int					i = 0;

	StdDeck_CardMask_RESET(hand);

	if (strlen(txt)) {
		while (txt[i*2] != 0) {
			card[0] = txt[0+i*2];
			card[1] = txt[1+i*2];
			card[2] = 0;
		
			// convert card to index value
			StdDeck_stringToCard(card, &index);
			// convert index value to mask
			currCard = StdDeck_MASK(index);
			// add the card to the hand
			StdDeck_CardMask_OR(hand, hand, currCard);
			// move to next card
			i++;
		}
	}

	return hand;
}


// Tidies up user input for a hand
void	cleanInput(char *hand) {
	int	i = 0;

	while (hand[i] != 0) {
		if (i%2 == 0)
			hand[i] = toupper(hand[i]);
		else
			hand[i] = tolower(hand[i]);

		i++;
	}
}
