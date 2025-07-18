/*
 -------------------------------------
 File:    deck.c
 Project: 3DS BlackJack
 Deck management implementation for blackjack game
 -------------------------------------
 Author:  Spencer Kelly
 Version  2025-07-17
 -------------------------------------
 */
#include "deck.h"
#include <string.h>

// Constants for card faces and suits
const char *cardFaces[] = { 
    "Ace", "Two", "Three", "Four", "Five", "Six",
    "Seven", "Eight", "Nine", "Ten", "Jack", "Queen", "King" 
};

// Using simple ASCII characters for 3DS compatibility
const char *cardSuits[] = { "Spades", "Hearts", "Clubs", "Diamonds" };
const char *cardSuitSymbols[] = { "S", "H", "C", "D" };

// Card sign arrays for display
const char *cardSigns[] = { 
    "A", "2", "3", "4", "5", "6", "7", "8", "9", "T", "J", "Q", "K" 
};

void init_decks(Deck *deck, int num_decks) {
    // Validate input - ensure reasonable number of decks
    if (num_decks < 1 || num_decks > MAX_DECKS) {
        printf("Error: Number of decks must be between 1 and %d\n", MAX_DECKS);
        return;
    }
    
    // Calculate total cards needed (52 cards per deck)
    int total_cards = num_decks * CARDS_PER_DECK;
    
    // Allocate memory for all cards in the deck(s)
    deck->cards = (Card*)malloc(total_cards * sizeof(Card));
    
    // Check if memory allocation was successful
    if (deck->cards == NULL) {
        printf("Error: Failed to allocate memory for deck\n");
        return;
    }
    
    // Initialize deck properties
    deck->size = total_cards;
    deck->cut_card_position = -1; // No cut card initially
    
    int deck_index = 0; // Index to track position in the cards array
    
    // Create each deck
    for (int d = 0; d < num_decks; d++) {
        // Create each face value (0-12, where 0=Ace, 1=Two, ..., 12=King)
        for (int f = 0; f < 13; f++) {
            // Create each suit (Spades, Hearts, Clubs, Diamonds)
            for (int s = 0; s < 4; s++) {
                // Set card face and suit from predefined arrays
                deck->cards[deck_index].face = cardFaces[f];
                deck->cards[deck_index].suit = cardSuits[s];
                deck->cards[deck_index].hidden = 0; // Default to visible
                
                // Copy card sign (display character) into the fixed array
                strcpy(deck->cards[deck_index].sign, cardSigns[f]);
                
                // Set card value and ace flag based on face value
                if (f == 0) { 
                    // Ace - starts as 11, can be adjusted to 1 later
                    deck->cards[deck_index].value = 11;
                    deck->cards[deck_index].is_ace = 1;
                } else if (f >= 10) { 
                    // Jack (10), Queen (11), King (12) - all worth 10 points
                    deck->cards[deck_index].value = 10;
                    deck->cards[deck_index].is_ace = 0;
                } else { 
                    // Number cards 2-10 (f=1 to f=9) - worth face value
                    deck->cards[deck_index].value = f + 1;
                    deck->cards[deck_index].is_ace = 0;
                }
                
                deck_index++; // Move to next position in array
            }
        }
    }
    
    printf("Initialized %d deck(s) with %d total cards\n", num_decks, total_cards);
}

void print_deck(const Deck *deck) {
    if (deck == NULL || deck->cards == NULL) {
        printf("Error: Invalid deck\n");
        return;
    }
    
    printf("Deck contents (%d cards):\n", deck->size);
    for (int i = 0; i < deck->size; i++) {
        printf("%d: %s of %s (Value: %d)\n", 
               i + 1, deck->cards[i].face, deck->cards[i].suit, deck->cards[i].value);
    }
}

void shuffle_deck(Deck *deck) {
    if (deck == NULL || deck->cards == NULL || deck->size == 0) {
        printf("Error: Cannot shuffle invalid deck\n");
        return;
    }
    
    // Use system time for random seed (3DS compatible)
    srand((unsigned int)time(NULL));
    
    // Fisher-Yates shuffle algorithm
    // This provides a mathematically fair shuffle
    for (int i = deck->size - 1; i > 0; i--) {
        // Pick a random index from 0 to i (inclusive)
        int j = rand() % (i + 1);
        
        // Swap cards at positions i and j
        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temp;
    }
    
    printf("Deck shuffled successfully using Fisher-Yates algorithm\n");
}

void check_and_shuffle(Deck *deck) {
    if (deck == NULL || deck->cards == NULL) {
        return;
    }
    
    // Check if we've reached the cut card position
    // In real blackjack, when cut card is reached, no more cards are dealt from current deck
    if (deck->cut_card_position >= 0 && deck->size <= deck->cut_card_position) {
        printf("Cut card reached! Getting fresh deck...\n");
        
        // Store the number of decks we were using
        int num_decks = (deck->size + (52 - (deck->size % 52))) / 52;
        if (deck->size % 52 != 0) {
            // Calculate original number of decks based on total cards that were in deck
            // This is an approximation - in a real game you'd track this separately
            num_decks = 1; // Default to single deck for simplicity
        }
        
        // Clean up current deck memory
        cleanup_deck(deck);
        
        // Initialize a completely fresh deck with same number of decks
        init_decks(deck, num_decks);
        
        // Shuffle the fresh deck
        shuffle_deck(deck);
        
        // Place new cut card in the fresh deck
        cut_card(deck);
        
        printf("Fresh deck ready with %d cards\n", deck->size);
    }
}

void cut_card(Deck *deck) {
    if (deck == NULL || deck->size == 0) {
        return;
    }
    
    // Place cut card somewhere in the last quarter to half of the deck
    // This determines when we'll need to get a fresh deck
    // In real casinos, cut card is usually placed about 1/4 to 1/2 from the bottom
    srand((unsigned int)time(NULL));
    
    // Place cut card between 1/4 and 1/2 from the bottom of deck
    int min_position = deck->size / 4;      // 25% from bottom
    int max_position = deck->size / 2;      // 50% from bottom
    deck->cut_card_position = rand() % (max_position - min_position + 1) + min_position;
    
    printf("Cut card placed - fresh deck needed when %d cards remain\n", 
           deck->cut_card_position);
}

Card deal_card(Deck *deck) {
    // Create a null card to return in case of error
    Card null_card = {NULL, NULL, "", 0, 0, 0};
    
    if (deck == NULL || deck->cards == NULL || deck->size == 0) {
        printf("Error: Cannot deal from empty or invalid deck\n");
        return null_card;
    }
    
    // IMPORTANT: Check if we've reached the cut card BEFORE dealing
    // In real blackjack, once cut card is reached, no more cards are dealt
    // from the current deck until a fresh deck is brought in
    if (deck->cut_card_position >= 0 && deck->size <= deck->cut_card_position) {
        printf("Cut card reached - cannot deal more cards from current deck\n");
        
        // Get a fresh deck and shuffle it
        check_and_shuffle(deck);
        
        // Now we can deal from the fresh deck
        if (deck->size == 0) {
            printf("Error: Fresh deck is empty\n");
            return null_card;
        }
    }
    
    // Deal from the top of the deck (last index in our array)
    // This simulates dealing from the top of a physical deck
    Card dealt_card = deck->cards[deck->size - 1];
    deck->size--; // Remove the dealt card from the deck
    
    return dealt_card;
}

void print_card(const Card card) {
    if (card.face == NULL || card.suit == NULL) {
        printf("Invalid card\n");
        return;
    }
    
    if (card.hidden) {
        printf("[Hidden Card]\n");
    } else {
        printf("%s of %s (%s) - Value: %d\n", 
               card.face, card.suit, card.sign, card.value);
    }
}

void cleanup_deck(Deck *deck) {
    if (deck == NULL) {
        return;
    }
    
    // Free the cards array (no need to free sign strings anymore)
    if (deck->cards != NULL) {
        free(deck->cards);
        deck->cards = NULL; // Prevent double-free
    }
    
    // Reset deck properties to safe state
    deck->size = 0;
    deck->cut_card_position = -1;
    
    printf("Deck memory cleaned up successfully\n");
}

int get_remaining_cards(const Deck *deck) {
    if (deck == NULL) {
        return 0;
    }
    return deck->size;
}
