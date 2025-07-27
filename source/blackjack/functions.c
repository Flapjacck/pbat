/*
 -------------------------------------
 File:    functions.c
 Project: 3DS BlackJack
 Game logic implementation for blackjack
 -------------------------------------
 Author:  Spencer Kelly
 Version  2025-07-17
 -------------------------------------
 */
#include "functions.h"
#include <string.h>

int game_start() {
    int cash = 500; // Default starting amount
    
    // Clear screen and display welcome message
    printf("\x1b[2J\x1b[H"); // Clear screen and move cursor to home
    printf("Welcome to 3DS BlackJack!\n");
    printf("=========================\n\n");
    
    // Get starting cash amount using up/down arrows
    printf("Select starting cash amount:\n");
    printf("Use UP/DOWN arrows to adjust (increments of $100)\n");
    printf("A = Confirm selection\n\n");
    
    // Display and update cash selection
    while (aptMainLoop()) {
        // Clear the specific lines where we display the cash amount
        printf("\x1b[6;1H\x1b[K"); // Move to line 6 and clear line
        printf("Starting cash: $%d", cash);
        printf("\x1b[7;1H\x1b[K"); // Clear line 7 too
        
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_DUP) {
            cash += 100; // Increment by $100
            if (cash > 10000) cash = 10000; // Cap at $10,000
        } else if (kDown & KEY_DDOWN) {
            cash -= 100; // Decrement by $100
            if (cash < 100) cash = 100; // Minimum $100
        } else if (kDown & KEY_A) {
            break; // Confirm selection
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    printf("\x1b[8;1H"); // Move to line 8
    printf("Starting with $%d\n\n", cash);
    return cash;
}

int place_bet(int cash) {
    // Calculate 5% increment (minimum $5)
    int increment = cash * 5 / 100;
    if (increment < 5) increment = 5;
    
    int bet = increment; // Start with one increment
    
    // Clear screen and show betting interface
    printf("\x1b[2J\x1b[H"); // Clear screen and move cursor to home
    printf("BETTING\n");
    printf("=======\n");
    printf("Current cash: $%d\n", cash);
    printf("Select bet amount:\n");
    printf("Use UP/DOWN arrows to adjust (increments of $%d - 5%% of cash)\n", increment);
    printf("A = Confirm bet\n\n");
    
    // Display and update bet selection
    while (aptMainLoop()) {
        // Clear the specific lines where we display bet info
        printf("\x1b[7;1H\x1b[K"); // Move to line 7 and clear line
        printf("Bet amount: $%d", bet);
        printf("\x1b[8;1H\x1b[K"); // Clear line 8
        printf("Remaining after bet: $%d", cash - bet);
        
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_DUP) {
            bet += increment; // Increment by 5% of cash
            if (bet > cash) bet = cash; // Cap at available cash
        } else if (kDown & KEY_DDOWN) {
            bet -= increment; // Decrement by 5% of cash
            if (bet < increment) bet = increment; // Minimum one increment
        } else if (kDown & KEY_A) {
            break; // Confirm selection
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    printf("\x1b[10;1H"); // Move to line 10
    printf("Bet placed: $%d\n\n", bet);
    return bet;
}

void deal_initial_cards(Deck *deck, Hand *player_hand, Hand *dealer_hand) {
    // Initialize hands
    player_hand->cards = malloc(10 * sizeof(Card)); // Max 10 cards per hand
    dealer_hand->cards = malloc(10 * sizeof(Card));
    
    // Reset hand values
    player_hand->num_cards = 0;
    player_hand->value = 0;
    player_hand->doubled = 0;
    player_hand->bust = 0;
    player_hand->natural_bj = 0;
    player_hand->stop = 0;
    strcpy(player_hand->name, "Player");
    
    dealer_hand->num_cards = 0;
    dealer_hand->value = 0;
    dealer_hand->doubled = 0;
    dealer_hand->bust = 0;
    dealer_hand->natural_bj = 0;
    dealer_hand->stop = 0;
    strcpy(dealer_hand->name, "Dealer");
    
    printf("Dealing initial cards...\n");
    printf("Debug: About to deal first card to player\n");
    
    // Deal first card to player
    add_card_to_hand(deck, player_hand);
    printf("Debug: First card dealt to player\n");
    
    // Deal first card to dealer (hidden)
    printf("Debug: About to deal first card to dealer\n");
    Card dealer_first = deal_card(deck);
    dealer_first.hidden = 1; // Hide dealer's first card
    dealer_hand->cards[dealer_hand->num_cards] = dealer_first;
    dealer_hand->num_cards++;
    printf("Dealer receives hidden card\n");
    
    // Deal second card to player
    printf("Debug: About to deal second card to player\n");
    add_card_to_hand(deck, player_hand);
    printf("Debug: Second card dealt to player\n");
    
    // Deal second card to dealer (visible)
    printf("Debug: About to deal second card to dealer\n");
    add_card_to_hand(deck, dealer_hand);
    printf("Debug: Second card dealt to dealer\n");
    
    // Calculate initial values
    calculate_hand_value(player_hand);
    calculate_hand_value(dealer_hand);
    
    // Check for natural blackjacks
    if (is_natural_blackjack(player_hand)) {
        player_hand->natural_bj = 1;
        printf("BLACKJACK! Player has 21!\n");
    }
}

int offer_insurance(int bet_amount, Hand *dealer_hand) {
    // Insurance only offered when dealer shows Ace
    if (dealer_hand->num_cards >= 2 && dealer_hand->cards[1].value == 11) {
        int insurance_bet = bet_amount / 2;
        
        select_player_screen();
        printf("Dealer shows Ace - Insurance available\n");
        printf("Insurance costs $%d (pays 2:1)\n", insurance_bet);
        printf("A = Take Insurance, B = Decline\n");
        
        while (aptMainLoop()) {
            hidScanInput();
            u32 kDown = hidKeysDown();
            
            if (kDown & KEY_A) {
                // Take insurance
                printf("Insurance taken for $%d\n", insurance_bet);
                
                // Check if dealer has blackjack (hidden card is 10-value)
                if (dealer_hand->cards[0].value == 10) {
                    dealer_hand->natural_bj = 1;
                    printf("Dealer has blackjack! Insurance pays $%d\n", insurance_bet * 2);
                    return insurance_bet * 2; // Return the winnings
                } else {
                    printf("Dealer does not have blackjack. Insurance lost.\n");
                    return -insurance_bet; // Return the loss (negative)
                }
            } else if (kDown & KEY_B) {
                printf("Insurance declined\n");
                
                // Still check if dealer has blackjack for game flow
                if (dealer_hand->cards[0].value == 10) {
                    dealer_hand->natural_bj = 1;
                }
                return 0;
            }
            
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }
    }
    return 0;
}

GameAction handle_player_turn(Deck *deck, Hand *player_hand, Hand *dealer_hand, int cash, int bet_amount) {
    while (!player_hand->stop && !is_busted(player_hand)) {
        printf("\nPlayer's turn:\n");
        
        // Check if player can double down (only on first turn with 2 cards)
        int can_double = (player_hand->num_cards == 2 && cash >= bet_amount);
        
        GameAction action = get_player_input(can_double);
        
        switch (action) {
            case ACTION_HIT:
                select_player_screen();
                printf("Player hits!\n");
                add_card_to_hand(deck, player_hand);
                calculate_hand_value(player_hand);
                
                // Refresh the entire game display with updated hands
                display_game_status_dual_screen(cash, bet_amount, player_hand, dealer_hand, 1);
                
                // Check for bust
                if (is_busted(player_hand)) {
                    player_hand->bust = 1;
                    player_hand->stop = 1;
                    select_player_screen();
                    printf("BUST! Player goes over 21!\n");
                }
                // Check for 6-card Charlie (instant win with 6 cards under 21)
                else if (player_hand->num_cards == 6) {
                    player_hand->stop = 1;
                    select_player_screen();
                    printf("6-Card Charlie! Player wins automatically!\n");
                }
                break;
                
            case ACTION_STAND:
                select_player_screen();
                printf("Player stands!\n");
                player_hand->stop = 1;
                break;
                
            case ACTION_DOUBLE:
                if (can_double) {
                    select_player_screen();
                    printf("Player doubles down!\n");
                    add_card_to_hand(deck, player_hand);
                    calculate_hand_value(player_hand);
                    player_hand->doubled = 1;
                    player_hand->stop = 1;
                    
                    // Refresh the entire game display with updated hands
                    display_game_status_dual_screen(cash, bet_amount * 2, player_hand, dealer_hand, 1);
                    
                    if (is_busted(player_hand)) {
                        player_hand->bust = 1;
                        select_player_screen();
                        printf("BUST! Player goes over 21!\n");
                    }
                } else {
                    select_player_screen();
                    printf("Cannot double down!\n");
                    continue; // Ask for input again
                }
                break;
                
            case ACTION_QUIT:
                return ACTION_QUIT;
                
            default:
                continue; // Invalid input, ask again
        }
        
        if (action != ACTION_HIT || player_hand->stop) {
            break;
        }
    }
    
    return ACTION_STAND; // Default return for completed turn
}

void calculate_hand_value(Hand *hand) {
    hand->value = 0;
    int aces = 0;
    
    // Calculate base value and count aces
    for (int i = 0; i < hand->num_cards; i++) {
        if (!hand->cards[i].hidden) { // Only count visible cards for value display
            hand->value += hand->cards[i].value;
            if (hand->cards[i].is_ace) {
                aces++;
            }
        }
    }
    
    // Adjust for soft aces (convert 11 to 1 if needed to avoid bust)
    while (hand->value > 21 && aces > 0) {
        hand->value -= 10; // Convert an ace from 11 to 1
        aces--;
    }
    
    // Set bust flag
    hand->bust = (hand->value > 21) ? 1 : 0;
}

void add_card_to_hand(Deck *deck, Hand *hand) {
    Card dealt_card = deal_card(deck);
    hand->cards[hand->num_cards] = dealt_card;
    hand->num_cards++;
    
    printf("%s receives: ", hand->name);
    print_card(dealt_card);
    
    // Recalculate hand value
    calculate_hand_value(hand);
}

void display_hand(Hand *hand) {
    printf("\n%s's hand:\n", hand->name);
    
    // Print card representations
    for (int i = 0; i < hand->num_cards; i++) {
        printf(" -----   ");
    }
    printf("\n");
    
    for (int i = 0; i < hand->num_cards; i++) {
        if (hand->cards[i].hidden) {
            printf("|  ?  |  ");
        } else {
            printf("| %-2s  |  ", hand->cards[i].sign);
        }
    }
    printf("\n");
    
    for (int i = 0; i < hand->num_cards; i++) {
        if (hand->cards[i].hidden) {
            printf("|  ?  |  ");
        } else {
            // Use ASCII characters instead of Unicode suits for 3DS compatibility
            char suit_char = hand->cards[i].suit[0]; // First letter of suit name
            printf("| %c   |  ", suit_char); // S, H, C, D for Spades, Hearts, Clubs, Diamonds
        }
    }
    printf("\n");
    
    for (int i = 0; i < hand->num_cards; i++) {
        if (hand->cards[i].hidden) {
            printf("|  ?  |  ");
        } else {
            printf("| %-2s  |  ", hand->cards[i].sign);
        }
    }
    printf("\n");
    
    for (int i = 0; i < hand->num_cards; i++) {
        printf(" -----   ");
    }
    printf("\n");
    
    // Show value (only visible cards for dealer if first card is hidden)
    if (strcmp(hand->name, "Dealer") == 0 && hand->cards[0].hidden) {
        printf("Showing: %d\n", hand->cards[1].value);
    } else {
        printf("Total: %d\n", hand->value);
    }
}

int determine_winner(Hand *player_hand, Hand *dealer_hand) {
    select_player_screen();
    
    // Player busts - dealer wins
    if (is_busted(player_hand)) {
        printf("Player busts! Dealer wins.\n");
        return -1;
    }
    
    // Dealer busts - player wins
    if (is_busted(dealer_hand)) {
        printf("Dealer busts! Player wins.\n");
        return 1;
    }
    
    // 6-card Charlie rule - player wins with 6 cards under 21
    if (player_hand->num_cards == 6 && !is_busted(player_hand)) {
        printf("6-Card Charlie! Player wins.\n");
        return 1;
    }
    
    // Compare values
    if (player_hand->value > dealer_hand->value) {
        printf("Player wins! %d beats %d\n", player_hand->value, dealer_hand->value);
        return 1;
    } else if (player_hand->value < dealer_hand->value) {
        printf("Dealer wins! %d beats %d\n", dealer_hand->value, player_hand->value);
        return -1;
    } else {
        printf("Push! Both have %d\n", player_hand->value);
        return 0;
    }
}

void handle_dealer_turn(Deck *deck, Hand *dealer_hand) {
    // Reveal dealer's hidden card
    dealer_hand->cards[0].hidden = 0;
    select_dealer_screen();
    printf("Dealer reveals hidden card:\n");
    calculate_hand_value(dealer_hand); // Recalculate with all cards visible
    display_hand_dual_screen(dealer_hand, GFX_TOP);
    
    // Dealer hits on 16 or less, stands on 17 or more
    while (dealer_hand->value < 17) {
        select_dealer_screen();
        printf("Dealer must hit (value: %d)\n", dealer_hand->value);
        add_card_to_hand(deck, dealer_hand);
        display_hand_dual_screen(dealer_hand, GFX_TOP);
        
        if (is_busted(dealer_hand)) {
            dealer_hand->bust = 1;
            break;
        }
    }
    
    if (!is_busted(dealer_hand)) {
        select_dealer_screen();
        printf("Dealer stands on %d\n", dealer_hand->value);
    }
}

void clear_hands(Hand *player_hand, Hand *dealer_hand) {
    // Free allocated memory only if it was allocated
    if (player_hand != NULL && player_hand->cards != NULL) {
        free(player_hand->cards);
        player_hand->cards = NULL;
    }
    if (dealer_hand != NULL && dealer_hand->cards != NULL) {
        free(dealer_hand->cards);
        dealer_hand->cards = NULL;
    }
    
    // Reset all values only if pointers are valid
    if (player_hand != NULL) {
        player_hand->num_cards = 0;
        player_hand->value = 0;
        player_hand->doubled = 0;
        player_hand->bust = 0;
        player_hand->natural_bj = 0;
        player_hand->stop = 0;
    }
    
    if (dealer_hand != NULL) {
        dealer_hand->num_cards = 0;
        dealer_hand->value = 0;
        dealer_hand->doubled = 0;
        dealer_hand->bust = 0;
        dealer_hand->natural_bj = 0;
        dealer_hand->stop = 0;
    }
}

int is_busted(Hand *hand) {
    return hand->value > 21;
}

int is_natural_blackjack(Hand *hand) {
    return (hand->num_cards == 2 && hand->value == 21);
}

GameAction get_player_input(int can_double) {
    select_player_screen();
    printf("\nChoose action:\n");
    printf("A = Hit, B = Stand");
    if (can_double) {
        printf(", X = Double Down");
    }
    printf("\nSTART = Quit\n");
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_A) {
            return ACTION_HIT;
        } else if (kDown & KEY_B) {
            return ACTION_STAND;
        } else if (kDown & KEY_X && can_double) {
            return ACTION_DOUBLE;
        } else if (kDown & KEY_START) {
            return ACTION_QUIT;
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    return ACTION_QUIT; // Fallback
}

void display_game_status(int cash, int bet_amount, Hand *player_hand, Hand *dealer_hand, int hide_dealer_card) {
    printf("\x1b[2J\x1b[H"); // Clear screen and move cursor to home
    printf("3DS BlackJack\n");
    printf("=============\n");
    printf("Cash: $%d | Bet: $%d\n\n", cash, bet_amount);
    
    // Display dealer hand
    if (hide_dealer_card && dealer_hand->num_cards > 0) {
        dealer_hand->cards[0].hidden = 1;
    }
    display_hand(dealer_hand);
    
    printf("\n");
    
    // Display player hand
    display_hand(player_hand);
}

// Global console variables for dual screen support
static PrintConsole g_topScreen, g_bottomScreen;
static int g_consoles_initialized = 0;

void init_dual_screen_consoles() {
    if (!g_consoles_initialized) {
        consoleInit(GFX_TOP, &g_topScreen);
        consoleInit(GFX_BOTTOM, &g_bottomScreen);
        g_consoles_initialized = 1;
    }
}

void select_player_screen() {
    init_dual_screen_consoles();
    consoleSelect(&g_bottomScreen);
}

void select_dealer_screen() {
    init_dual_screen_consoles();
    consoleSelect(&g_topScreen);
}

void clear_screen(gfxScreen_t screen) {
    init_dual_screen_consoles();
    
    if (screen == GFX_TOP) {
        consoleSelect(&g_topScreen);
    } else {
        consoleSelect(&g_bottomScreen);
    }
    
    printf("\x1b[2J\x1b[H"); // Clear screen and move cursor to home
}

void display_hand_dual_screen(Hand *hand, gfxScreen_t screen) {
    init_dual_screen_consoles();
    
    if (screen == GFX_TOP) {
        consoleSelect(&g_topScreen);
    } else {
        consoleSelect(&g_bottomScreen);
    }
    
    printf("\n%s's hand:\n", hand->name);
    
    // Print card representations
    for (int i = 0; i < hand->num_cards; i++) {
        printf(" -----   ");
    }
    printf("\n");
    
    for (int i = 0; i < hand->num_cards; i++) {
        if (hand->cards[i].hidden) {
            printf("|  ?  |  ");
        } else {
            printf("| %-2s  |  ", hand->cards[i].sign);
        }
    }
    printf("\n");
    
    for (int i = 0; i < hand->num_cards; i++) {
        if (hand->cards[i].hidden) {
            printf("|  ?  |  ");
        } else {
            // Use ASCII characters instead of Unicode suits for 3DS compatibility
            char suit_char = hand->cards[i].suit[0]; // First letter of suit name
            printf("| %c   |  ", suit_char); // S, H, C, D for Spades, Hearts, Clubs, Diamonds
        }
    }
    printf("\n");
    
    for (int i = 0; i < hand->num_cards; i++) {
        if (hand->cards[i].hidden) {
            printf("|  ?  |  ");
        } else {
            printf("| %-2s  |  ", hand->cards[i].sign);
        }
    }
    printf("\n");
    
    for (int i = 0; i < hand->num_cards; i++) {
        printf(" -----   ");
    }
    printf("\n");
    
    // Show value (only visible cards for dealer if first card is hidden)
    if (strcmp(hand->name, "Dealer") == 0 && hand->cards[0].hidden) {
        printf("Showing: %d\n", hand->cards[1].value);
    } else {
        printf("Total: %d\n", hand->value);
    }
}

void display_game_status_dual_screen(int cash, int bet_amount, Hand *player_hand, Hand *dealer_hand, int hide_dealer_card) {
    init_dual_screen_consoles();
    
    // Clear both screens
    clear_screen(GFX_TOP);
    clear_screen(GFX_BOTTOM);
    
    // Display dealer on top screen
    consoleSelect(&g_topScreen);
    printf("3DS BlackJack - DEALER\n");
    printf("======================\n\n");
    
    // Display dealer hand on top screen
    if (hide_dealer_card && dealer_hand->num_cards > 0) {
        dealer_hand->cards[0].hidden = 1;
    }
    display_hand_dual_screen(dealer_hand, GFX_TOP);
    
    // Display player on bottom screen
    consoleSelect(&g_bottomScreen);
    printf("3DS BlackJack - PLAYER\n");
    printf("=======================\n");
    printf("Cash: $%d | Bet: $%d\n\n", cash, bet_amount);
    
    // Display player hand on bottom screen
    display_hand_dual_screen(player_hand, GFX_BOTTOM);
}
