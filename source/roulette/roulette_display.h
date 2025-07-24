/*
 * Roulette Display Functions Header for Nintendo 3DS
 * 
 * This file contains function declarations for all display-related operations
 * including wheel visualization, history display, and betting menu.
 */

#ifndef ROULETTE_DISPLAY_H
#define ROULETTE_DISPLAY_H

#include "roulette_types.h"

/*
 * Display the roulette wheel with visual representation
 * Shows the winning number highlighted with appropriate color coding
 * Displays the traditional roulette table layout with 0 at top and 3x12 grid below
 * 
 * @param winning_number: The number that won this spin (0-36)
 */
void display_wheel(int winning_number);

/*
 * Display the recent winning number history with color coding
 * Shows up to the last 10 numbers to help players track patterns
 * 
 * @param game: Pointer to the game state containing history data
 */
void display_history(GameState* game);

/*
 * Display the main betting menu with all available options
 * Shows controls and current chip count to help players make decisions
 */
void show_betting_menu(void);

/*
 * Display all currently placed bets with their details
 * Shows bet type, specific number (for straight bets), and chip amount
 * 
 * @param game: Pointer to the game state
 */
void display_bets(GameState* game);

#endif // ROULETTE_DISPLAY_H
