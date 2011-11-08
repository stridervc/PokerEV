/*
 * pokerev.c
 *
 * Calculates Poker EV
 *
 * Copyright 2011 Fred Strauss
 *
 * This program is distributed under the terms of the GNU General Public License.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define VERSION "0.1.0"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>

#include "poker_defs.h"
#include "inlines/eval.h"

static const char *optString = "Oh?";		// command line arguments we support
static const struct option longOpts[] = {	// long versions of options
    { "odds", no_argument, NULL, 'O' },
    { "help", no_argument, NULL, 'h' },
	{ NULL, no_argument, NULL, 0 }
};

void 	evalSingleTrial(int numHands, StdDeck_CardMask hands[], StdDeck_CardMask userBoard, StdDeck_CardMask board, 
			double wins[], double ties[], int *numberOfTrials);
StdDeck_CardMask	txtToMask(const char *txt);
void	cleanInput(char *hand);
void	display_help(char *progname);
void	display_version();

int main(int argc, char **argv) {

	char 				handstr[10][10];	// Array of hands read from user input (max 10)
	char				boardstr[10];		// Comunity cards read from user input
	StdDeck_CardMask	hands[10], board;	// CardMask versions of user input
	StdDeck_CardMask 	deadCards;			// Cards that shouldn't be in the deck
	int					numHands;			// Number of hands the user supplied
	int					i;					// The std looping variable
	int					opt;				// For getopt
	int					longIndex;			// For getopt
	bool				showOdds = false;	// Display odds instead of percentages?

	// Process command line arguments
	opt = getopt_long(argc, argv, optString, longOpts, &longIndex);

	while (opt != -1) {
		switch (opt) {
			case 'O':	// display odds instead of percentages
				showOdds = true;
				break;

			case 'h':
			case '?':
				display_help(argv[0]);
				return(0);

			default:
				break;
		}

		opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
	}

	// Keep reading hands from the user until done, or until we reach 10 hands
	i = 0;
	while (i < 10) {
		printf("Hand %d ", i+1);
		if (i >= 2)
			printf("(. for none) ");
		printf(": ");

		scanf("%s", handstr[i]);

		if (handstr[i][0] == '.')
			break;
		cleanInput(handstr[i]);
		hands[i] = txtToMask(handstr[i]);

		i++;
	}
	numHands = i;

	// Ask the user for community cards
	StdDeck_CardMask_RESET(board);
	printf("Board (. for none) : ");
	scanf("%s", boardstr);
	if (boardstr[0] != '.') {
		cleanInput(boardstr);
		board = txtToMask(boardstr);
	}

	// remove community cards and player hands from the deck
	StdDeck_CardMask_RESET(deadCards);
	for (i = 0; i < numHands; i++)
		StdDeck_CardMask_OR(deadCards, deadCards, hands[i]);

	// Keep stats
	double 	wins[10] = { 0.0 };
	double 	ties[10] = { 0.0 };
	int 	numberOfTrials = 0;

	// Enumerate all possible boards
	StdDeck_CardMask boardCards;
	if (StdDeck_numCards(board) == 0) {			// is there any board?
		DECK_ENUMERATE_5_CARDS_D(StdDeck, boardCards, deadCards, 
			evalSingleTrial(numHands, hands, board, boardCards, wins, ties, &numberOfTrials); );
	} else if (StdDeck_numCards(board) == 3) {	// is there a flop?
		DECK_ENUMERATE_2_CARDS_D(StdDeck, boardCards, deadCards, 
			evalSingleTrial(numHands, hands, board, boardCards, wins, ties, &numberOfTrials); );
	} else if (StdDeck_numCards(board) == 4) {	// is there a flop and turn?
		DECK_ENUMERATE_1_CARDS_D(StdDeck, boardCards, deadCards, 
			evalSingleTrial(numHands, hands, board, boardCards, wins, ties, &numberOfTrials); );
	} else {
		printf("Invalid board\r\n");
		return 1;
	}

	// Convert wins to equity
	double	handEquity[10];
	double	handWins[10];
	double	handTies[10];
	for (i = 0; i < numHands; i++) {
		handEquity[i] = ((wins[i] + ties[i]/2) / numberOfTrials) * 100.0;
		handWins[i] = (wins[i] / numberOfTrials) * 100.0;
		handTies[i] = (ties[i] / numberOfTrials) * 100.0;
	}

	printf("\r\n");
	display_version();

	if (StdDeck_numCards(board))
		printf("Board : %s\r\n\r\n", boardstr);

	if (showOdds)
		printf("       Equity                 : Win                    : Tie \r\n");
	else
		printf("       Equity    : Win       : Tie \r\n");

	for (i = 0; i < numHands; i++)
		if (showOdds) {	// convert percentages to odds, maybe more efficient to do this above, but then we need another array to store dog / favourite
			double	equityOdds, winOdds;
			bool	dog = false;

			if (handEquity[i] < 50) {		// we're a dog
				dog = true;
				equityOdds = (100-handEquity[i]) / handEquity[i];
				winOdds = (100-handWins[i]) / handWins[i];

			} else {						// we're the favourite
				equityOdds = handEquity[i] / (100-handEquity[i]);
				winOdds = handWins[i] / (100-handWins[i]);
			}

			printf("%s : %0.4f %% (%0.2f:1 %s) : %0.4f %% (%0.2f:1 %s) : %0.4f %%\r\n", handstr[i], handEquity[i], equityOdds, dog?"dog":"fav", handWins[i], winOdds, dog?"dog":"fav", handTies[i]);

		} else {	// show percentages
			printf("%s : %0.4f %% : %0.4f %% : %0.4f %%\r\n", handstr[i], handEquity[i], handWins[i], handTies[i]);
		}

	return 0;
}

/*
 * Evaluates a single enumeration of possible communit cards
 * numHands = Number of hands involved
 * hand = array of all hands involved
 * userBoard = community cards as supplied by the user
 * board = current enumeration of the rest of the board
 * wins = keep track of wins for each hand
 * ties = keep track of ties for each hand
 * numberOfTrials = keep track of the number of enumerations gone through
*/
void evalSingleTrial(int numHands, StdDeck_CardMask hands[], StdDeck_CardMask userBoard, StdDeck_CardMask board, 
		double wins[], double ties[], int *numberOfTrials) {
	
	StdDeck_CardMask	tmpHand;
	int					i;
	int					handVal[10];
	int					maxVal = -1;	// the best hand value
	int 				tie = 0;		// do we have a tie?

	// Combine each player's hole cards with board cards
	for (i = 0; i < numHands; i++) {
		StdDeck_CardMask_RESET(tmpHand);
		StdDeck_CardMask_OR(tmpHand, hands[i], board);
		StdDeck_CardMask_OR(tmpHand, tmpHand, userBoard);

		// evaluate hand strength
		handVal[i] = StdDeck_StdRules_EVAL_N(tmpHand, 7);
	
		// is this hand better than the best so far?
		if (handVal[i] > maxVal) {
			maxVal = handVal[i];
			tie = 0;
		
		// or does it equal the max hand? iow we have a tie
		} else if (handVal[i] == maxVal) {
			tie = 1;
		}


	}

	// chalk up one for the winner, or one for each tied hand
	for (i = 0; i < numHands; i++) {
		if (handVal[i] == maxVal)
			if (tie)
				ties[i] += 1.0;
			else
				wins[i] += 1.0;
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

// Display help text
void	display_help( char *progname) {
	display_version();
	printf("Usage: %s [OPTION]...\r\n\r\n", progname);
	printf("OPTION can be:\r\n");
	//printf("\t-o, --omaha\tCalculate odds for Omaha\r\n");
	printf("\t-O, --odds\tDisplay odds as well as percentages\r\n");
	printf("\t-h, -?, --help\tHelp\r\n");
	printf("\r\n");
}

void	display_version() {
	printf("PokerEV version %s\r\n", VERSION);
	printf("\r\n");
}
