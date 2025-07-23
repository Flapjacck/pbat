/*
 * Roulette Game for Nintendo 3DS
 * 
 * This file implements a European roulette game with the following features:
 * - 37 numbers (0-36) with authentic wheel layout
 * - Multiple betting options (straight, color, dozen, line, range, parity)
 * - Visual wheel display with color coding
 * - Betting history tracking
 * - Chip management system
 */

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/*
 * European roulette wheel layout (37 numbers: 0-36)
 * Numbers are arranged in the actual order they appear on a European roulette wheel
 * Starting from 0 and going clockwise around the wheel
 */
static const int wheel_numbers[] = {
    0, 32, 15, 19, 4, 21, 2, 25, 17, 34, 6, 27, 13, 36, 11, 30, 8, 23, 10, 5, 24, 16, 33, 1, 20, 14, 31, 9, 22, 18, 29, 7, 28, 12, 35, 3, 26
};

/*
 * Color mapping for each position on the wheel
 * 0 = green (only for 0), 1 = red, 2 = black
 * Corresponds to the traditional European roulette color scheme
 */
static const int wheel_colors[] = {
    0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2
};

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
 * Add a winning number to the game history
 * Maintains a rolling history of the last 15 spins
 * 
 * @param game: Pointer to the game state
 * @param number: The winning number to add to history (0-36)
 */
void add_to_history(GameState* game, int number) {
    if (game->history_count < 15) {
        // Still have space in history array
        game->last_numbers[game->history_count] = number;
        game->history_count++;
    } else {
        // History is full, shift all numbers left and add new one at end
        for (int i = 0; i < 14; i++) {
            game->last_numbers[i] = game->last_numbers[i + 1];
        }
        game->last_numbers[14] = number;
    }
}

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
        return (bet_number == winning_number) ? bet_amount * 35 : 0;
    } 
    // Color bets: red or black (pays 1:1, even money)
    else if (strcmp(bet_type, "RED") == 0) {
        return (is_red(winning_number)) ? bet_amount * 1 : 0;
    } else if (strcmp(bet_type, "BLACK") == 0) {
        return (is_black(winning_number)) ? bet_amount * 1 : 0;
    } 
    // Parity bets: even or odd numbers (pays 1:1, even money)
    // Note: 0 is neither even nor odd for betting purposes
    else if (strcmp(bet_type, "EVEN") == 0) {
        return (winning_number > 0 && winning_number % 2 == 0) ? bet_amount * 1 : 0;
    } else if (strcmp(bet_type, "ODD") == 0) {
        return (winning_number > 0 && winning_number % 2 == 1) ? bet_amount * 1 : 0;
    } 
    // Range bets: low (1-18) or high (19-36) numbers (pays 1:1, even money)
    else if (strcmp(bet_type, "LOW") == 0) {
        return (winning_number >= 1 && winning_number <= 18) ? bet_amount * 1 : 0;
    } else if (strcmp(bet_type, "HIGH") == 0) {
        return (winning_number >= 19 && winning_number <= 36) ? bet_amount * 1 : 0;
    } 
    // Dozen bets: groups of 12 numbers (pays 2:1)
    else if (strcmp(bet_type, "1ST12") == 0) {
        return (winning_number >= 1 && winning_number <= 12) ? bet_amount * 2 : 0;
    } else if (strcmp(bet_type, "2ND12") == 0) {
        return (winning_number >= 13 && winning_number <= 24) ? bet_amount * 2 : 0;
    } else if (strcmp(bet_type, "3RD12") == 0) {
        return (winning_number >= 25 && winning_number <= 36) ? bet_amount * 2 : 0;
    } 
    // Line bets: vertical columns of numbers (pays 2:1)
    // 1L: numbers ending in 1 (1,4,7,10,13,16,19,22,25,28,31,34)
    // 2L: numbers ending in 2 (2,5,8,11,14,17,20,23,26,29,32,35)  
    // 3L: numbers ending in 0 (3,6,9,12,15,18,21,24,27,30,33,36)
    else if (strcmp(bet_type, "1L") == 0) {
        return (winning_number > 0 && winning_number % 3 == 1) ? bet_amount * 2 : 0;
    } else if (strcmp(bet_type, "2L") == 0) {
        return (winning_number > 0 && winning_number % 3 == 2) ? bet_amount * 2 : 0;
    } else if (strcmp(bet_type, "3L") == 0) {
        return (winning_number > 0 && winning_number % 3 == 0) ? bet_amount * 2 : 0;
    }
    
    return 0; // Unknown bet type or losing bet
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

// Get bet amount from user using D-pad
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

// Get number from user (0-36) using D-pad
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

int roulette_main(int argc, char* argv[])
{
    // Graphics already initialized by main menu
    consoleInit(GFX_TOP, NULL);
    
    // Initialize random seed
    srand(time(NULL));
    
    // Initialize game state
    GameState game = {0};
    game.chips = 100; // Starting chips
    game.num_bets = 0;
    game.history_count = 0;
    
    printf("\x1b[2J");  // Clear screen
    printf("\x1b[1;1H"); // Move cursor to top-left
    
    printf("========================================\n");
    printf("        WELCOME TO ROULETTE!            \n");
    printf("========================================\n");
    printf("\n");
    printf("European Roulette - 37 numbers (0-36)\n");
    printf("Starting chips: %d\n", game.chips);
    printf("\n");
    printf("Press A to start playing!\n");
    printf("Press START to return to main menu\n");
    
    // Wait for initial input
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_A) {
            break;
        }
        if (kDown & KEY_START) {
            return 0;
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    // Main game loop
    while (aptMainLoop() && game.chips > 0) {
        printf("\x1b[2J\x1b[1;1H");
        show_betting_menu();
        printf("%d\n", game.chips);
        display_bets(&game);
        display_history(&game);
        
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_START) {
            // Spin the wheel if we have bets
            if (game.num_bets > 0) {
                printf("\nSpinning the wheel...\n");
                
                // Animation delay
                for (int i = 0; i < 10; i++) {
                    printf(".");
                    gfxFlushBuffers();
                    gfxSwapBuffers();
                    for (int j = 0; j < 10; j++) {
                        gspWaitForVBlank();
                    }
                }
                
                int winning_number = spin_wheel();
                add_to_history(&game, winning_number);
                
                display_wheel(winning_number);
                
                // Calculate winnings
                int total_winnings = 0;
                printf("\nResults:\n");
                for (int i = 0; i < game.num_bets; i++) {
                    int winnings = calculate_winnings(game.bet_numbers[i], game.bet_types[i], game.bet_amounts[i], winning_number);
                    if (winnings > 0) {
                        printf("WIN: %s bet won %d chips!\n", game.bet_types[i], winnings);
                        total_winnings += winnings;
                    }
                }
                
                game.chips += total_winnings;
                
                printf("\nTotal winnings: %d chips\n", total_winnings);
                printf("Chips remaining: %d\n", game.chips);
                
                // Clear bets for next round
                game.num_bets = 0;
                
                printf("\nPress A to continue...\n");
                while (aptMainLoop()) {
                    hidScanInput();
                    u32 kDown2 = hidKeysDown();
                    if (kDown2 & KEY_A) break;
                    gfxFlushBuffers();
                    gfxSwapBuffers();
                    gspWaitForVBlank();
                }
            } else {
                printf("\nPlace at least one bet first!\n");
                for (int i = 0; i < 30; i++) {
                    gspWaitForVBlank();
                }
            }
        } else if (kDown & KEY_SELECT) {
            // Clear all bets
            clear_roulette_bets(&game);
            printf("\nAll bets cleared!\n");
            for (int i = 0; i < 30; i++) {
                gspWaitForVBlank();
            }
        } else if (kDown & KEY_A) {
            // Place straight bet
            printf("\nSelect number (0-36):\n");
            int number = get_number_input();
            if (number >= 0) {
                int amount = get_bet_amount();
                if (amount > 0 && amount <= game.chips) {
                    place_roulette_bet(&game, "STRAIGHT", number, amount);
                }
            }
        } else if (kDown & KEY_B) {
            // Place color bet
            printf("\nSelect color (UP=RED, DOWN=BLACK):\n");
            while (aptMainLoop()) {
                hidScanInput();
                u32 kDown2 = hidKeysDown();
                if (kDown2 & KEY_UP) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "RED", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_DOWN) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "BLACK", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_B) {
                    break;
                }
                gfxFlushBuffers();
                gfxSwapBuffers();
                gspWaitForVBlank();
            }
        } else if (kDown & KEY_X) {
            // Place dozen bet
            printf("\nSelect dozen (LEFT=1ST, UP=2ND, RIGHT=3RD):\n");
            while (aptMainLoop()) {
                hidScanInput();
                u32 kDown2 = hidKeysDown();
                if (kDown2 & KEY_LEFT) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "1ST12", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_UP) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "2ND12", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_RIGHT) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "3RD12", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_B) {
                    break;
                }
                gfxFlushBuffers();
                gfxSwapBuffers();
                gspWaitForVBlank();
            }
        } else if (kDown & KEY_Y) {
            // Place line bet
            printf("\nSelect line (LEFT=1L, UP=2L, RIGHT=3L):\n");
            while (aptMainLoop()) {
                hidScanInput();
                u32 kDown2 = hidKeysDown();
                if (kDown2 & KEY_LEFT) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "1L", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_UP) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "2L", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_RIGHT) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "3L", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_B) {
                    break;
                }
                gfxFlushBuffers();
                gfxSwapBuffers();
                gspWaitForVBlank();
            }
        } else if (kDown & KEY_L) {
            // Place range bet
            printf("\nSelect range (UP=HIGH 19-36, DOWN=LOW 1-18):\n");
            while (aptMainLoop()) {
                hidScanInput();
                u32 kDown2 = hidKeysDown();
                if (kDown2 & KEY_UP) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "HIGH", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_DOWN) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "LOW", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_B) {
                    break;
                }
                gfxFlushBuffers();
                gfxSwapBuffers();
                gspWaitForVBlank();
            }
        } else if (kDown & KEY_R) {
            // Place parity bet
            printf("\nSelect parity (UP=EVEN, DOWN=ODD):\n");
            while (aptMainLoop()) {
                hidScanInput();
                u32 kDown2 = hidKeysDown();
                if (kDown2 & KEY_UP) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "EVEN", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_DOWN) {
                    int amount = get_bet_amount();
                    if (amount > 0 && amount <= game.chips) {
                        place_roulette_bet(&game, "ODD", 0, amount);
                    }
                    break;
                } else if (kDown2 & KEY_B) {
                    break;
                }
                gfxFlushBuffers();
                gfxSwapBuffers();
                gspWaitForVBlank();
            }
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    // Game over screen
    if (game.chips <= 0) {
        printf("\x1b[2J\x1b[1;1H");
        printf("========================================\n");
        printf("             GAME OVER!                 \n");
        printf("========================================\n");
        printf("\n");
        printf("You have run out of chips!\n");
        printf("Better luck next time!\n");
        printf("\n");
        printf("Press START to return to main menu...\n");
        
        while (aptMainLoop()) {
            hidScanInput();
            u32 kDown = hidKeysDown();
            
            if (kDown & KEY_START) {
                break;
            }
            
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }
    }
    
    return 0;
}