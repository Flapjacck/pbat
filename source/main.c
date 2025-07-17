#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include "deck.h"

int main(int argc, char* argv[])
{
    // Initialize graphics
    gfxInitDefault();
    
    // Initialize console for top screen
    consoleInit(GFX_TOP, NULL);
    
    printf("3DS BlackJack - Deck Test\n");
    printf("========================\n\n");
    
    // Create and test the deck
    Deck deck;
    int num_decks = 1;
    
    printf("Initializing %d deck(s)...\n", num_decks);
    init_decks(&deck, num_decks);
    
    printf("Shuffling deck...\n");
    shuffle_deck(&deck);
    
    printf("Setting cut card...\n");
    cut_card(&deck);
    
    printf("\nDealing first 5 cards:\n");
    printf("----------------------\n");
    
    for (int i = 0; i < 5 && get_remaining_cards(&deck) > 0; i++) {
        Card dealt = deal_card(&deck);
        printf("Card %d: ", i + 1);
        print_card(dealt);
    }
    
    printf("\nCards remaining in deck: %d\n", get_remaining_cards(&deck));
    
    printf("\nPress START to exit, A to deal another card\n");
    
    // Main loop
    while (aptMainLoop())
    {
        // Scan for user input
        hidScanInput();
        
        // Get current key states
        u32 kDown = hidKeysDown();
        
        // Exit when START is pressed
        if (kDown & KEY_START)
            break;
        
        // Deal another card when A is pressed
        if (kDown & KEY_A) {
            if (get_remaining_cards(&deck) > 0) {
                Card dealt = deal_card(&deck);
                printf("Dealt: ");
                print_card(dealt);
                printf("Cards remaining: %d\n", get_remaining_cards(&deck));
            } else {
                printf("No more cards in deck!\n");
            }
        }
        
        // Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        
        // Wait for VBlank
        gspWaitForVBlank();
    }
    
    // Clean up
    cleanup_deck(&deck);
    
    // Exit graphics
    gfxExit();
    return 0;
}
