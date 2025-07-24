/*
 * Roulette Types Header for Nintendo 3DS
 * 
 * This file contains all the data structures and constants used in the roulette game.
 * Includes the game state structure, wheel layout, and color mappings.
 */

#ifndef ROULETTE_TYPES_H
#define ROULETTE_TYPES_H

/*
 * European roulette wheel layout (37 numbers: 0-36)
 * Numbers are arranged in the actual order they appear on a European roulette wheel
 * Starting from 0 and going clockwise around the wheel
 */
extern const int wheel_numbers[37];

/*
 * Color mapping for each position on the wheel
 * 0 = green (only for 0), 1 = red, 2 = black
 * Corresponds to the traditional European roulette color scheme
 */
extern const int wheel_colors[37];

/*
 * Game state structure to track all roulette game data
 * Contains player's chips, current bets, and game history
 */
typedef struct {
    int chips;                    // Player's current chip count
    int bet_numbers[10];          // Numbers for straight bets (0-36)
    int bet_amounts[10];          // Amount wagered on each bet
    char bet_types[10][20];       // Type of each bet (e.g., "STRAIGHT", "RED", etc.)
    int num_bets;                 // Current number of active bets (max 10)
    int last_numbers[15];         // History of the last 15 winning numbers
    int history_count;            // Number of spins in history (0-15)
} GameState;

#endif // ROULETTE_TYPES_H
