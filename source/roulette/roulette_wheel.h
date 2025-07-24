/*
 * Roulette Wheel Functions Header for Nintendo 3DS
 * 
 * This file contains function declarations for wheel-related operations including
 * wheel position calculations, color checking, and spinning mechanics.
 */

#ifndef ROULETTE_WHEEL_H
#define ROULETTE_WHEEL_H

#include "roulette_types.h"

/*
 * Find the index position of a given number on the roulette wheel
 * 
 * @param number: The number to find (0-36)
 * @return: Index in the wheel_numbers array, or -1 if not found
 */
int get_wheel_index(int number);

/*
 * Simulate spinning the roulette wheel to generate a random winning number
 * Uses multiple random calls for better randomization to simulate wheel physics
 * 
 * @return: Random number from 0-36 representing the winning number
 */
int spin_wheel(void);

/*
 * Check if a given number is red on the roulette wheel
 * 
 * @param number: The number to check (0-36)
 * @return: 1 if the number is red, 0 if black or green (0)
 */
int is_red(int number);

/*
 * Check if a given number is black on the roulette wheel
 * 
 * @param number: The number to check (0-36)
 * @return: 1 if the number is black, 0 if red or green (0)
 */
int is_black(int number);

#endif // ROULETTE_WHEEL_H
