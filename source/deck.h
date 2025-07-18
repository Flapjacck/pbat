/*
 -------------------------------------
 File:    deck.h
 Project: 3DS BlackJack
 Deck management for blackjack game
 -------------------------------------
 Author:  Spencer Kelly
 Version  2025-07-17
 -------------------------------------
 */
#ifndef DECK_H_
#define DECK_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <3ds.h>

// Constants
#define MAX_DECKS 8
#define CARDS_PER_DECK 52
#define MAX_TOTAL_CARDS (MAX_DECKS * CARDS_PER_DECK)

// Struct for a card
typedef struct {
	const char *face;
	const char *suit;
	char sign[3]; // Letter symbol (A, 2-9, T, J, Q, K) - fixed size array
	int value; // Value of card
	int is_ace; // 1 if ace, 0 if not
	int hidden; // 1 if hidden, 0 if visible
} Card;

// Struct for the deck
typedef struct {
	Card *cards;
	int size; // Current number of cards in deck
	int cut_card_position; // Position of cut card for reshuffling
} Deck;

// Function prototypes

// Function to initialize multiple decks of cards.
// Parameters:
// - deck: Pointer to the deck structure
// - num_decks: Number of decks to initialize (1-8)
void init_decks(Deck *deck, int num_decks);

// Function to print a deck of cards to console (for debugging).
// Parameters:
// - deck: Pointer to the deck structure
void print_deck(const Deck *deck);

// Function to shuffle a deck of cards using Fisher-Yates algorithm.
// Parameters:
// - deck: Pointer to the deck structure
void shuffle_deck(Deck *deck);

// Function to check if the cut card is reached and get a fresh deck if necessary.
// In real blackjack, when the cut card is reached, no more cards are dealt from
// the current deck. Instead, a completely fresh deck is brought in, shuffled,
// and a new cut card is placed.
// Parameters:
// - deck: Pointer to the deck structure
void check_and_shuffle(Deck *deck);

// Function to place a cut card in the deck at a random position.
// The cut card determines when a fresh deck is needed. Typically placed
// about 1/4 to 1/2 from the bottom of the deck in real casinos.
// Parameters:
// - deck: Pointer to the deck structure
void cut_card(Deck *deck);

// Function to deal a card from the deck.
// Automatically checks for cut card and gets fresh deck if needed.
// Cards are dealt from the "top" of the deck (last index in array).
// Parameters:
// - deck: Pointer to the deck structure
// Returns the dealt card, or a null card if error occurs.
Card deal_card(Deck *deck);

// Function to print a single card to console.
// Parameters:
// - card: The card to print
void print_card(const Card card);

// Function to clean up allocated memory for the deck.
// Parameters:
// - deck: Pointer to the deck structure
void cleanup_deck(Deck *deck);

// Function to get the number of cards remaining in deck.
// Parameters:
// - deck: Pointer to the deck structure
// Returns the number of cards remaining
int get_remaining_cards(const Deck *deck);

#endif /* DECK_H_ */
