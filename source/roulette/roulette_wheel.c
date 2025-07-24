/*
 * Roulette Wheel Functions for Nintendo 3DS
 * 
 * This file implements wheel-related operations including wheel position calculations,
 * color checking, and spinning mechanics for the European roulette game.
 */

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "roulette_wheel.h"

/*
 * European roulette wheel layout (37 numbers: 0-36)
 * Numbers are arranged in the actual order they appear on a European roulette wheel
 * Starting from 0 and going clockwise around the wheel
 */
const int wheel_numbers[] = {
    0, 32, 15, 19, 4, 21, 2, 25, 17, 34, 6, 27, 13, 36, 11, 30, 8, 23, 10, 5, 24, 16, 33, 1, 20, 14, 31, 9, 22, 18, 29, 7, 28, 12, 35, 3, 26
};

/*
 * Color mapping for each position on the wheel
 * 0 = green (only for 0), 1 = red, 2 = black
 * Corresponds to the traditional European roulette color scheme
 */
const int wheel_colors[] = {
    0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2
};

/*
 * Find the index position of a given number on the roulette wheel
 * 
 * @param number: The number to find (0-36)
 * @return: Index in the wheel_numbers array, or -1 if not found
 */
int get_wheel_index(int number) {
    for (int i = 0; i < 37; i++) {
        if (wheel_numbers[i] == number) {
            return i;
        }
    }
    return -1; // Should never happen with valid roulette numbers
}

/*
 * Simulate spinning the roulette wheel to generate a random winning number
 * Uses multiple random calls for better randomization to simulate wheel physics
 * 
 * @return: Random number from 0-36 representing the winning number
 */
int spin_wheel() {
    // Use multiple random calls for better randomization
    // This simulates the unpredictability of a physical wheel spin
    for (int i = 0; i < rand() % 10 + 5; i++) {
        rand();
    }
    return rand() % 37; // Return number between 0-36 inclusive
}

/*
 * Check if a given number is red on the roulette wheel
 * 
 * @param number: The number to check (0-36)
 * @return: 1 if the number is red, 0 if black or green (0)
 */
int is_red(int number) {
    if (number == 0) return 0; // 0 is green, not red
    int index = get_wheel_index(number);
    return wheel_colors[index] == 1;
}

/*
 * Check if a given number is black on the roulette wheel
 * 
 * @param number: The number to check (0-36)
 * @return: 1 if the number is black, 0 if red or green (0)
 */
int is_black(int number) {
    if (number == 0) return 0; // 0 is green, not black
    int index = get_wheel_index(number);
    return wheel_colors[index] == 2;
}
