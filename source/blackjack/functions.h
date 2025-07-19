/*
 -------------------------------------
 File:    functions.h
 Project: 3DS BlackJack
 Game logic and hand management for blackjack
 -------------------------------------
 Author:  Spencer Kelly
 Version  2025-07-17
 -------------------------------------
 */
#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>
#include "deck.h"

// Hand structure to represent player and dealer hands
typedef struct {
	Card *cards;        // Array of cards in the hand
	int num_cards;      // Number of cards currently in hand
	int value;          // Current total value of the hand
	int doubled;        // 1 if player doubled down, 0 if not
	int bust;           // 1 if hand is busted (over 21), 0 if not
	int natural_bj;     // 1 if natural blackjack (21 with 2 cards), 0 if not
	char name[20];      // Name identifier for the hand ("Player" or "Dealer")
	int stop;           // 1 if no more cards should be dealt, 0 if can continue
} Hand;

// Game action enumeration for cleaner code
typedef enum {
    ACTION_HIT = 0,
    ACTION_STAND = 1,
    ACTION_DOUBLE = 2,
    ACTION_INSURANCE = 3,
    ACTION_QUIT = 4
} GameAction;

// Function prototypes

// Initializes the game and prompts for starting cash amount using UP/DOWN arrows
// Uses increments of $100, ranging from $100 to $10,000
// Returns the starting cash amount
int game_start();

// Handles betting using UP/DOWN arrows for amount selection
// Uses increments of 5% of current cash (minimum $5)
// Parameters:
// - cash: Current amount of cash the player has
// Returns the bet amount
int place_bet(int cash);

// Deals the initial two cards to both player and dealer
// First dealer card is hidden, second is visible
// Parameters:
// - deck: Pointer to the deck to deal from
// - player_hand: Pointer to player's hand structure
// - dealer_hand: Pointer to dealer's hand structure
void deal_initial_cards(Deck *deck, Hand *player_hand, Hand *dealer_hand);

// Offers insurance when dealer shows an Ace
// Insurance pays 2:1 if dealer has blackjack
// Parameters:
// - bet_amount: Original bet amount (insurance is half of this)
// - dealer_hand: Dealer's hand to check for blackjack
// Returns insurance payout (0 if lost, bet_amount if won)
int offer_insurance(int bet_amount, Hand *dealer_hand);

// Handles player's turn choices using 3DS buttons
// Player can hit, stand, or double down
// Parameters:
// - deck: Deck to deal cards from
// - player_hand: Player's hand
// - cash: Current cash (for double down validation)
// - bet_amount: Current bet amount
// Returns the action taken
GameAction handle_player_turn(Deck *deck, Hand *player_hand, Hand *dealer_hand, int cash, int bet_amount);

// Calculates and updates hand value, handling Ace soft/hard values
// Aces count as 11 unless that would bust, then they count as 1
// Parameters:
// - hand: Hand to calculate value for
void calculate_hand_value(Hand *hand);

// Deals a card to a hand and updates the hand's state
// Parameters:
// - deck: Deck to deal from
// - hand: Hand to add card to
void add_card_to_hand(Deck *deck, Hand *hand);

// Displays the cards in a hand with ASCII art representation
// Hidden cards show as face-down
// Parameters:
// - hand: Hand to display
void display_hand(Hand *hand);

// Determines the winner and returns the result
// Parameters:
// - player_hand: Player's final hand
// - dealer_hand: Dealer's final hand
// Returns: 1 = player wins, -1 = player loses, 0 = push (tie)
int determine_winner(Hand *player_hand, Hand *dealer_hand);

// Handles dealer's turn according to standard blackjack rules
// Dealer hits on 16 and below, stands on 17 and above
// Parameters:
// - deck: Deck to deal from
// - dealer_hand: Dealer's hand
void handle_dealer_turn(Deck *deck, Hand *dealer_hand);

// Clears and resets both hands for a new round
// Frees allocated memory and resets all values
// Parameters:
// - player_hand: Player's hand to clear
// - dealer_hand: Dealer's hand to clear
void clear_hands(Hand *player_hand, Hand *dealer_hand);

// Checks if a hand is busted (over 21)
// Parameters:
// - hand: Hand to check
// Returns 1 if busted, 0 if not
int is_busted(Hand *hand);

// Checks if a hand is a natural blackjack (21 with exactly 2 cards)
// Parameters:
// - hand: Hand to check
// Returns 1 if natural blackjack, 0 if not
int is_natural_blackjack(Hand *hand);

// Gets player input using 3DS buttons with visual prompts
// Shows which buttons to press for different actions
// Returns the action chosen by the player
GameAction get_player_input(int can_double);

// Displays game status including cash, bet, and current hands
// Parameters:
// - cash: Current cash amount
// - bet_amount: Current bet amount
// - player_hand: Player's hand
// - dealer_hand: Dealer's hand
// - hide_dealer_card: Whether to hide dealer's first card
void display_game_status(int cash, int bet_amount, Hand *player_hand, Hand *dealer_hand, int hide_dealer_card);

#endif /* FUNCTIONS_H_ */
