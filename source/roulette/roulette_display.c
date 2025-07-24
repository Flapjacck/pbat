/*
 * Roulette Display Functions for Nintendo 3DS
 * 
 * This file implements all display-related operations including wheel visualization,
 * history display, and betting menu for the European roulette game.
 */

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "roulette_display.h"
#include "roulette_wheel.h"

/*
 * Display the roulette wheel with visual representation
 * Shows the winning number highlighted with appropriate color coding
 * Displays the traditional roulette table layout with 0 at top and 3x12 grid below
 * 
 * @param winning_number: The number that won this spin (0-36)
 */
void display_wheel(int winning_number) {
    printf("\x1b[2J\x1b[1;1H"); // Clear screen and move cursor to top-left
    printf("========================================\n");
    printf("           ROULETTE WHEEL               \n");
    printf("========================================\n\n");
    
    // Display the winning number with color coding
    printf("Winning Number: ");
    if (winning_number == 0) {
        printf("\x1b[42m\x1b[30m %d \x1b[0m", winning_number); // Green background for 0
    } else if (is_red(winning_number)) {
        printf("\x1b[41m\x1b[37m %d \x1b[0m", winning_number); // Red background for red numbers
    } else {
        printf("\x1b[40m\x1b[37m %d \x1b[0m", winning_number); // Black background for black numbers
    }
    printf("\n\n");
    
    // Display the 0 pocket at the top (traditional roulette layout)
    printf("  0  ");
    if (winning_number == 0) printf("\x1b[42m\x1b[30m"); // Highlight if 0 won
    printf(" 0 ");
    if (winning_number == 0) printf("\x1b[0m"); // Reset color
    printf("\n\n");
    
    // Display the main betting grid: 3 rows of 12 columns each
    // Numbers 1-36 arranged as they appear on a roulette table
    for (int row = 3; row >= 1; row--) {
        for (int col = 1; col <= 12; col++) {
            int num = col * 3 - (3 - row); // Calculate number for this position
            if (num == winning_number) {
                // Highlight winning number with appropriate color
                if (is_red(num)) {
                    printf("\x1b[41m\x1b[37m"); // Red background
                } else {
                    printf("\x1b[40m\x1b[37m"); // Black background
                }
            }
            printf("%2d ", num);
            if (num == winning_number) {
                printf("\x1b[0m"); // Reset color formatting
            }
        }
        printf("  %dL\n", row); // Line bet indicator (e.g., "1L", "2L", "3L")
    }
    
    printf("\n");
    // Display the dozen betting areas
    printf("   F12      S12      T12   \n");
    printf(" (1-12)  (13-24)  (25-36) \n");
}

/*
 * Display the recent winning number history with color coding
 * Shows up to the last 10 numbers to help players track patterns
 * 
 * @param game: Pointer to the game state containing history data
 */
void display_history(GameState* game) {
    printf("\nHistory (last %d spins): ", game->history_count);
    
    // Show only the last 10 numbers to avoid screen clutter
    int start = (game->history_count > 10) ? game->history_count - 10 : 0;
    
    for (int i = start; i < game->history_count; i++) {
        int num = game->last_numbers[i];
        // Color code each number according to roulette colors
        if (num == 0) {
            printf("\x1b[42m\x1b[30m %d \x1b[0m", num); // Green for 0
        } else if (is_red(num)) {
            printf("\x1b[41m\x1b[37m %d \x1b[0m", num); // Red background
        } else {
            printf("\x1b[40m\x1b[37m %d \x1b[0m", num); // Black background
        }
        printf(" ");
    }
    printf("\n");
}

/*
 * Display the main betting menu with all available options
 * Shows controls and current chip count to help players make decisions
 */
void show_betting_menu() {
    printf("\n========================================\n");
    printf("            BETTING OPTIONS             \n");
    printf("========================================\n");
    printf("A - Place straight bet (0-36, pays 35:1)\n");
    printf("B - Place color bet (RED/BLACK, pays 1:1)\n");
    printf("X - Place dozen bet (1ST/2ND/3RD, pays 2:1)\n");
    printf("Y - Place line bet (1L/2L/3L, pays 2:1)\n");
    printf("L - Place range bet (LOW/HIGH, pays 1:1)\n");
    printf("R - Place parity bet (EVEN/ODD, pays 1:1)\n");
    printf("START - Spin wheel\n");
    printf("SELECT - Clear all bets\n");
    printf("\nCurrent chips: ");
}

/*
 * Display all currently placed bets with their details
 * Shows bet type, specific number (for straight bets), and chip amount
 * 
 * @param game: Pointer to the game state
 */
void display_bets(GameState* game) {
    if (game->num_bets == 0) {
        printf("No bets placed.\n");
        return;
    }
    
    printf("\nCurrent bets:\n");
    for (int i = 0; i < game->num_bets; i++) {
        printf("%d. %s", i + 1, game->bet_types[i]);
        // Show the specific number for straight bets
        if (strcmp(game->bet_types[i], "STRAIGHT") == 0) {
            printf(" (%d)", game->bet_numbers[i]);
        }
        printf(" - %d chips\n", game->bet_amounts[i]);
    }
}
