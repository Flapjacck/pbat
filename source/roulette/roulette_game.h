/*
 * Roulette Game Functions Header for Nintendo 3DS
 * 
 * This file contains function declarations for game state management
 * and history tracking operations.
 */

#ifndef ROULETTE_GAME_H
#define ROULETTE_GAME_H

#include "roulette_types.h"

/*
 * Add a winning number to the game history
 * Maintains a rolling history of the last 15 spins
 * 
 * @param game: Pointer to the game state
 * @param number: The winning number to add to history (0-36)
 */
void add_to_history(GameState* game, int number);

/*
 * Main roulette game function
 * 
 * @param argc: Argument count
 * @param argv: Argument values
 * @return: Exit code
 */
int roulette_main(int argc, char* argv[]);

#endif // ROULETTE_GAME_H
