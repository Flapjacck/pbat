/*
 * Roulette Betting Functions Header for Nintendo 3DS
 * 
 * This file contains function declarations for betting operations including
 * bet placement, input handling, and winnings calculation.
 */

#ifndef ROULETTE_BETTING_H
#define ROULETTE_BETTING_H

#include "roulette_types.h"

/*
 * Calculate winnings for a specific bet based on the winning number
 * Implements all standard European roulette bet types and their payouts
 * 
 * @param bet_number: The number bet on (only used for straight bets)
 * @param bet_type: String describing the type of bet
 * @param bet_amount: Amount of chips wagered
 * @param winning_number: The number that won this spin
 * @return: Total winnings (original bet + profit), or 0 if bet lost
 */
int calculate_winnings(int bet_number, const char* bet_type, int bet_amount, int winning_number);

/*
 * Place a new bet in the game state
 * Validates that player has enough chips and bet limit hasn't been reached
 * 
 * @param game: Pointer to the game state
 * @param bet_type: Type of bet being placed (e.g., "STRAIGHT", "RED")
 * @param bet_number: Number for straight bets (ignored for other bet types)
 * @param amount: Number of chips to wager
 */
void place_roulette_bet(GameState* game, const char* bet_type, int bet_number, int amount);

/*
 * Clear all currently placed bets and return chips to player
 * Useful when player wants to start over with their betting strategy
 * 
 * @param game: Pointer to the game state
 */
void clear_roulette_bets(GameState* game);

/*
 * Get bet amount from user using D-pad
 * 
 * @return: Selected bet amount (1-50), or 0 if cancelled
 */
int get_bet_amount(void);

/*
 * Get number from user (0-36) using D-pad
 * 
 * @return: Selected number (0-36), or -1 if cancelled
 */
int get_number_input(void);

#endif // ROULETTE_BETTING_H
