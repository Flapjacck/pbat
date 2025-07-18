#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "deck.h"
#include "functions.h"

int main(int argc, char* argv[])
{
    // Initialize graphics and console
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    
    // Game variables
    int cash = 0;
    int bet_amount = 0;
    int insurance_payout = 0;
    Hand player_hand;
    Hand dealer_hand;
    Deck deck;
    char play_again = 'y';
    
    // Initialize the game
    cash = game_start();
    
    // Ask for number of decks using up/down arrows
    printf("\x1b[2J\x1b[H"); // Clear screen and move cursor to home
    printf("DECK SELECTION\n");
    printf("==============\n");
    printf("Select number of decks:\n");
    printf("Use UP/DOWN arrows to adjust (1-8 decks)\n");
    printf("A = Confirm selection\n\n");
    
    int num_decks = 1; // Start with 1 deck
    while (aptMainLoop()) {
        // Clear the specific line where we display deck count
        printf("\x1b[6;1H\x1b[K"); // Move to line 6 and clear line
        printf("Number of decks: %d", num_decks);
        
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_DUP) {
            num_decks++; // Increment by 1
            if (num_decks > 8) num_decks = 8; // Cap at 8 decks
        } else if (kDown & KEY_DDOWN) {
            num_decks--; // Decrement by 1
            if (num_decks < 1) num_decks = 1; // Minimum 1 deck
        } else if (kDown & KEY_A) {
            break; // Confirm selection
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    // Initialize and shuffle deck
    printf("\x1b[8;1H"); // Move to line 8
    init_decks(&deck, num_decks);
    shuffle_deck(&deck);
    cut_card(&deck);
    printf("Deck ready with %d deck(s)\n", num_decks);
    printf("Press A to start playing...\n");
    
    // Wait for user to continue
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_A) {
            break;
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    // Main game loop
    while (cash > 0 && play_again == 'y') {
        printf("\n========================================\n");
        printf("New Round - Cash: $%d\n", cash);
        printf("========================================\n");
        
        // Clear hands from previous round
        clear_hands(&player_hand, &dealer_hand);
        
        // Place bet
        bet_amount = place_bet(cash);
        cash -= bet_amount;
        
        // Deal initial cards
        deal_initial_cards(&deck, &player_hand, &dealer_hand);
        
        // Display initial game state
        display_game_status(cash, bet_amount, &player_hand, &dealer_hand, 1);
        
        // Check for insurance opportunity
        insurance_payout = offer_insurance(bet_amount, &dealer_hand);
        cash += insurance_payout; // Apply insurance result to cash
        
        // Check for natural blackjacks
        int player_natural = is_natural_blackjack(&player_hand);
        int dealer_natural = is_natural_blackjack(&dealer_hand);
        
        if (player_natural || dealer_natural) {
            // Reveal dealer's hidden card for blackjack resolution
            dealer_hand.cards[0].hidden = 0;
            calculate_hand_value(&dealer_hand);
            display_game_status(cash, bet_amount, &player_hand, &dealer_hand, 0);
            
            if (player_natural && dealer_natural) {
                printf("Both have Blackjack! Push!\n");
                cash += bet_amount; // Return bet
            } else if (player_natural) {
                printf("Player Blackjack! Pays 3:2\n");
                cash += bet_amount + (bet_amount * 3 / 2); // 3:2 payout
            } else {
                printf("Dealer Blackjack! Player loses.\n");
                // Player loses bet (already deducted)
            }
        } else {
            // Normal gameplay - player's turn
            GameAction result = handle_player_turn(&deck, &player_hand, &dealer_hand, cash, bet_amount);
            
            if (result == ACTION_QUIT) {
                break; // Exit game
            }
            
            // Handle double down
            if (player_hand.doubled) {
                cash -= bet_amount;
                bet_amount *= 2;
                printf("Bet doubled to $%d\n", bet_amount);
            }
            
            // Dealer's turn (only if player didn't bust)
            if (!is_busted(&player_hand)) {
                printf("\nDealer's turn:\n");
                handle_dealer_turn(&deck, &dealer_hand);
            }
            
            // Display final hands
            display_game_status(cash, bet_amount, &player_hand, &dealer_hand, 0);
            
            // Determine winner and pay out
            int result_code = determine_winner(&player_hand, &dealer_hand);
            
            if (result_code == 1) {
                // Player wins
                if (player_hand.num_cards == 6) {
                    // 6-card Charlie pays 2:1
                    cash += bet_amount * 3;
                    printf("6-Card Charlie pays 2:1! Won $%d\n", bet_amount * 2);
                } else {
                    // Normal win pays 1:1
                    cash += bet_amount * 2;
                    printf("Won $%d\n", bet_amount);
                }
            } else if (result_code == 0) {
                // Push - return bet
                cash += bet_amount;
            }
            // If result_code == -1, player loses (bet already deducted)
        }
        
        printf("\nCurrent cash: $%d\n", cash);
        
        // Check if player is out of money
        if (cash <= 0) {
            printf("Out of money! Game Over.\n");
            break;
        }
        
        // Ask if player wants to play another round
        printf("\nPlay another round?\n");
        printf("A = Yes, B = No\n");
        
        while (aptMainLoop()) {
            hidScanInput();
            u32 kDown = hidKeysDown();
            
            if (kDown & KEY_A) {
                play_again = 'y';
                break;
            } else if (kDown & KEY_B) {
                play_again = 'n';
                break;
            }
            
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }
    }
    
    // Game over
    printf("\n========================================\n");
    printf("Game Over!\n");
    printf("Final cash: $%d\n", cash);
    printf("Thanks for playing 3DS BlackJack!\n");
    printf("Press START to exit.\n");
    printf("========================================\n");
    
    // Wait for player to exit
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
    
    // Clean up
    clear_hands(&player_hand, &dealer_hand);
    cleanup_deck(&deck);
    
    // Exit graphics
    gfxExit();
    return 0;
}
