/*
 * pokerev.c
 *
 * Calculates Poker EV
 *
 * Copyright 2011-2012 Fred Strauss
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

#define VERSION "0.2.0"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>

#include "poker_defs.h"
#include "inlines/eval.h"

void 	evalSingleTrial(StdDeck_CardMask player1, StdDeck_CardMask player2, StdDeck_CardMask userBoard, StdDeck_CardMask board, 
			double wins[], double ties[], int *numberOfTrials);
StdDeck_CardMask	txtToMask(const char *txt);
void	cleanInput(char *hand);
void	display_help(char *progname);
void	display_version();

int main(int argc, char **argv) {

	char 				hand1str[10];
	char				hand2str[10];
	char				boardstr[10];
	StdDeck_CardMask	hand1, hand2, board;
	int					i;					// The std looping variable
	bool				handsSupplied = false;
	bool				explicitInteractive = false;
	bool				outputCSV = false;

	// Command line arguments
	static const char *optString = "hi?c";		// short options
	static const struct option longOpts[] = {	// long options
	    { "help", no_argument, NULL, 'h' },
	    { "version", no_argument, NULL, 0 },
	    { "csv", no_argument, NULL, 'c' },
	    { "hands", required_argument, NULL, 'H' },
	    { "board", required_argument, NULL, 'b' },
	    { NULL, no_argument, NULL, 0 }
	};
	int					opt;				// For getopt
	int					longIndex;			// For getopt

	StdDeck_CardMask_RESET(board);	// clear board var

	// Process command line arguments
	opt = getopt_long(argc, argv, optString, longOpts, &longIndex);

	while (opt != -1) {
		switch (opt) {
			case 'h':
			case '?':
				display_help(argv[0]);
				return(0);

			// board cards supplied
			case 'b':
				if (optarg) {
					strcpy(boardstr,optarg);
					cleanInput(boardstr);
					board = txtToMask(boardstr);
				}
				break;

			// player hands supplied
			case 'H':
				if (optarg) {
					strcpy(hand1str, strtok(optarg,":"));
					strcpy(hand2str, strtok(NULL,":"));
					handsSupplied = true;

					cleanInput(hand1str);
					hand1 = txtToMask(hand1str);
					cleanInput(hand2str);
					hand2 = txtToMask(hand2str);
				}
				break;

			// force interactive
			case 'i':
				explicitInteractive = true;
				break;

			// output as csv
			case 'c':
				outputCSV = true;
				break;

			// display version
			case 0:
				if (strcmp("version",longOpts[longIndex].name) == 0) {
					display_version();
					return 0;
				}
			
			default:
				break;
		}

		opt = getopt_long(argc, argv, optString, longOpts, &longIndex);
	}

	// prompt for hands if not supplied on command line
	if (!handsSupplied) {
		printf("Hand 1 : ");
		fgets(hand1str,10,stdin);
		cleanInput(hand1str);
		hand1 = txtToMask(hand1str);

		printf("Hand 2 : ");
		fgets(hand2str,10,stdin);
		cleanInput(hand2str);
		hand2 = txtToMask(hand2str);
	}

	// prompt for board if it wasn't supplied on command line
	if (!handsSupplied || explicitInteractive) {
		StdDeck_CardMask_RESET(board);
		printf("Board (ENTER for none) : ");
		if (fgets(boardstr,10,stdin)) {
			cleanInput(boardstr);
			board = txtToMask(boardstr);
		} else {
			boardstr[0] = 0;
		}
	}

	int cardIndex = -1;

	// Dead cards
	StdDeck_CardMask deadCards;
	StdDeck_CardMask_RESET(deadCards);
	StdDeck_CardMask_OR(deadCards, hand1, hand2);
	StdDeck_CardMask_OR(deadCards, deadCards, board);

	// Keep stats
	double wins[2] = { 0.0 };
	double ties[2] = { 0.0 };
	int numberOfTrials = 0;

	// Enumerate all possible boards
	StdDeck_CardMask boardCards;
	if (StdDeck_numCards(board) == 0) {			// is there any board?
		DECK_ENUMERATE_5_CARDS_D(StdDeck, boardCards, deadCards, 
			evalSingleTrial(hand1, hand2, board, boardCards, wins, ties, &numberOfTrials); );
	} else if (StdDeck_numCards(board) == 3) {	// is there a flop?
		DECK_ENUMERATE_2_CARDS_D(StdDeck, boardCards, deadCards, 
			evalSingleTrial(hand1, hand2, board, boardCards, wins, ties, &numberOfTrials); );
	} else if (StdDeck_numCards(board) == 4) {	// is there a flop and turn?
		DECK_ENUMERATE_1_CARDS_D(StdDeck, boardCards, deadCards, 
			evalSingleTrial(hand1, hand2, board, boardCards, wins, ties, &numberOfTrials); );
	} else {
		printf("Invalid board\r\n");
		return 1;
	}

	// Convert wins to equity
	double h1Equity = ((wins[0] + ties[0]/2) / numberOfTrials) * 100.0;
	double h2Equity = ((wins[1] + ties[1]/2) / numberOfTrials) * 100.0;
	double h1Wins = (wins[0] / numberOfTrials) * 100.0;
	double h2Wins = (wins[1] / numberOfTrials) * 100.0;
	double h1Ties = (ties[0] / numberOfTrials) * 100.0;
	double h2Ties = (ties[1] / numberOfTrials) * 100.0;

	// normal output
	if (!outputCSV) {
		printf("\r\n");
		display_version();

		if (StdDeck_numCards(board))
			printf("Board : %s\r\n\r\n", boardstr);
		else
			printf("Board : None\r\n\r\n");

		printf("       Equity     : Win        : Tie \r\n");
		printf("%s : %8.4f %% : %8.4f %% : %8.4f %%\r\n", hand1str, h1Equity, h1Wins, h1Ties);
		printf("%s : %8.4f %% : %8.4f %% : %8.4f %%\r\n", hand2str, h2Equity, h2Wins, h2Ties);

	// CSV output
	} else {
		printf("# hand,equity %%,win %,tie %%\r\n");
		printf("%s,%.4f,%.4f,%.4f\r\n", hand1str, h1Equity, h1Wins, h1Ties);
		printf("%s,%.4f,%.4f,%.4f\r\n", hand2str, h2Equity, h2Wins, h2Ties);
	}

	return 0;
}

void evalSingleTrial(StdDeck_CardMask player1, StdDeck_CardMask player2, StdDeck_CardMask userBoard, StdDeck_CardMask board, 
	double wins[], double ties[], int *numberOfTrials) {
	// Combine each player's hole cards with board cards
	StdDeck_CardMask_OR(player1, player1, board);
	StdDeck_CardMask_OR(player1, player1, userBoard);

	StdDeck_CardMask_OR(player2, player2, board);
	StdDeck_CardMask_OR(player2, player2, userBoard);

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
		if (hand[i] == '\n') {
			hand[i] = 0;
			break;
		}

		if (i%2 == 0)
			hand[i] = toupper(hand[i]);
		else
			hand[i] = tolower(hand[i]);

		i++;
	}
}

// Display help text
void	display_help(char *progname) {
	display_version();
	printf("Calculates equity for poker hands.\r\n\r\n");
	printf("Usage: %s [OPTION]\r\n\r\n", progname);
	printf("OPTION can be:\r\n");
	printf("\t--hands CARDS\tProvide player cards on command line, separate players with :\r\n");
	printf("\t--board CARDS\tProvide board (flop,[turn]) on command line\r\n");
	printf("\t-i\t\tInteractive. Prompt for board even if --hands is supplied\r\n");
	printf("\t-c, --csv\tOutput as csv\r\n");
	printf("\t-h, -?, --help\tHelp\r\n");
	printf("\t--version\tDisplay version and exit\r\n");
	printf("\r\n");
}

void	display_version() {
	printf("PokerEV version %s\r\n", VERSION);
}
