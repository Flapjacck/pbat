/*
 * Roulette Game Functions for Nintendo 3DS
 * 
 * This file implements the main game loop and game state management
 * for the European roulette game.
 */

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "roulette_game.h"
#include "roulette_display.h"
#include "roulette_betting.h"
#include "roulette_wheel.h"

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
 * Main roulette game function
 * 
 * @param argc: Argument count
 * @param argv: Argument values
 * @return: Exit code
 */
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
