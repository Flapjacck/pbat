/*
 * Roulette Betting Functions for Nintendo 3DS
 * 
 * This file implements betting operations including bet placement, input handling,
 * and winnings calculation for the European roulette game.
 */

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "roulette_betting.h"
#include "roulette_wheel.h"

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
int calculate_winnings(int bet_number, const char* bet_type, int bet_amount, int winning_number) {
    // Straight bet: betting on a single number (pays 35:1)
    if (strcmp(bet_type, "STRAIGHT") == 0) {
        return (bet_number == winning_number) ? bet_amount * 36 : 0;
    } 
    // Color bets: red or black (pays 1:1, even money)
    else if (strcmp(bet_type, "RED") == 0) {
        return (is_red(winning_number)) ? bet_amount * 2 : 0;
    } else if (strcmp(bet_type, "BLACK") == 0) {
        return (is_black(winning_number)) ? bet_amount * 2 : 0;
    } 
    // Parity bets: even or odd numbers (pays 1:1, even money)
    // Note: 0 is neither even nor odd for betting purposes
    else if (strcmp(bet_type, "EVEN") == 0) {
        return (winning_number > 0 && winning_number % 2 == 0) ? bet_amount * 2 : 0;
    } else if (strcmp(bet_type, "ODD") == 0) {
        return (winning_number > 0 && winning_number % 2 == 1) ? bet_amount * 2 : 0;
    } 
    // Range bets: low (1-18) or high (19-36) numbers (pays 1:1, even money)
    else if (strcmp(bet_type, "LOW") == 0) {
        return (winning_number >= 1 && winning_number <= 18) ? bet_amount * 2 : 0;
    } else if (strcmp(bet_type, "HIGH") == 0) {
        return (winning_number >= 19 && winning_number <= 36) ? bet_amount * 2 : 0;
    } 
    // Dozen bets: groups of 12 numbers (pays 2:1)
    else if (strcmp(bet_type, "1ST12") == 0) {
        return (winning_number >= 1 && winning_number <= 12) ? bet_amount * 3 : 0;
    } else if (strcmp(bet_type, "2ND12") == 0) {
        return (winning_number >= 13 && winning_number <= 24) ? bet_amount * 3 : 0;
    } else if (strcmp(bet_type, "3RD12") == 0) {
        return (winning_number >= 25 && winning_number <= 36) ? bet_amount * 3 : 0;
    } 
    // Line bets: vertical columns of numbers (pays 2:1)
    // 1L: numbers ending in 1 (1,4,7,10,13,16,19,22,25,28,31,34)
    // 2L: numbers ending in 2 (2,5,8,11,14,17,20,23,26,29,32,35)  
    // 3L: numbers ending in 0 (3,6,9,12,15,18,21,24,27,30,33,36)
    else if (strcmp(bet_type, "1L") == 0) {
        return (winning_number > 0 && winning_number % 3 == 1) ? bet_amount * 3 : 0;
    } else if (strcmp(bet_type, "2L") == 0) {
        return (winning_number > 0 && winning_number % 3 == 2) ? bet_amount * 3 : 0;
    } else if (strcmp(bet_type, "3L") == 0) {
        return (winning_number > 0 && winning_number % 3 == 0) ? bet_amount * 3 : 0;
    }
    
    return 0; // Unknown bet type or losing bet
}

/*
 * Place a new bet in the game state
 * Validates that player has enough chips and bet limit hasn't been reached
 * 
 * @param game: Pointer to the game state
 * @param bet_type: Type of bet being placed (e.g., "STRAIGHT", "RED")
 * @param bet_number: Number for straight bets (ignored for other bet types)
 * @param amount: Number of chips to wager
 */
void place_roulette_bet(GameState* game, const char* bet_type, int bet_number, int amount) {
    // Check if we can place another bet (max 10 bets) and player has enough chips
    if (game->num_bets < 10 && game->chips >= amount) {
        game->chips -= amount; // Deduct chips immediately when bet is placed
        game->bet_numbers[game->num_bets] = bet_number;
        game->bet_amounts[game->num_bets] = amount;
        strcpy(game->bet_types[game->num_bets], bet_type);
        game->num_bets++;
    }
}

/*
 * Clear all currently placed bets and return chips to player
 * Useful when player wants to start over with their betting strategy
 * 
 * @param game: Pointer to the game state
 */
void clear_roulette_bets(GameState* game) {
    // Return all wagered chips to player's balance
    for (int i = 0; i < game->num_bets; i++) {
        game->chips += game->bet_amounts[i];
    }
    game->num_bets = 0; // Reset bet counter
}

/*
 * Get bet amount from user using D-pad
 * 
 * @return: Selected bet amount (1-50), or 0 if cancelled
 */
int get_bet_amount() {
    int amount = 1;
    printf("Use UP/DOWN to select bet amount (1-50):\n");
    printf("Current amount: %d\n", amount);
    printf("Press A to confirm, B to cancel\n");
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_UP && amount < 50) {
            amount++;
            printf("\x1b[2A\x1b[KCurrent amount: %d\n", amount);
            printf("Press A to confirm, B to cancel\n");
        }
        if (kDown & KEY_DOWN && amount > 1) {
            amount--;
            printf("\x1b[2A\x1b[KCurrent amount: %d\n", amount);
            printf("Press A to confirm, B to cancel\n");
        }
        if (kDown & KEY_A) {
            return amount;
        }
        if (kDown & KEY_B) {
            return 0; // Cancel
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    return 0;
}

/*
 * Get number from user (0-36) using D-pad
 * 
 * @return: Selected number (0-36), or -1 if cancelled
 */
int get_number_input() {
    int number = 0;
    printf("Use LEFT/RIGHT for tens, UP/DOWN for ones:\n");
    printf("Current number: %d\n", number);
    printf("Press A to confirm, B to cancel\n");
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_RIGHT && number < 30) {
            number += 10;
            if (number > 36) number = 36;
            printf("\x1b[2A\x1b[KCurrent number: %d\n", number);
            printf("Press A to confirm, B to cancel\n");
        }
        if (kDown & KEY_LEFT && number >= 10) {
            number -= 10;
            if (number < 0) number = 0;
            printf("\x1b[2A\x1b[KCurrent number: %d\n", number);
            printf("Press A to confirm, B to cancel\n");
        }
        if (kDown & KEY_UP && number < 36) {
            number++;
            printf("\x1b[2A\x1b[KCurrent number: %d\n", number);
            printf("Press A to confirm, B to cancel\n");
        }
        if (kDown & KEY_DOWN && number > 0) {
            number--;
            printf("\x1b[2A\x1b[KCurrent number: %d\n", number);
            printf("Press A to confirm, B to cancel\n");
        }
        if (kDown & KEY_A) {
            return number;
        }
        if (kDown & KEY_B) {
            return -1; // Cancel
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    return -1;
}
