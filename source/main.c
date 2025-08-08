#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>

// Function prototypes for the games
extern int blackjack_main(int argc, char* argv[]);
extern int roulette_main(int argc, char* argv[]);
extern void text_editor_main(void);

void show_menu() {
    printf("\x1b[2J");  // Clear screen
    printf("\x1b[1;1H"); // Move cursor to top-left
    printf("========================================\n");
    printf("         PBAT CASINO GAMES              \n");
    printf("========================================\n");
    printf("\n");
    printf("Select a game:\n");
    printf("\n");
    printf("A - Blackjack\n");
    printf("B - Roulette\n");
    printf("Y - Text Editor\n");
    printf("X - Exit\n");
    printf("\n");
    printf("Press the corresponding button...\n");
}

int main(int argc, char* argv[])
{
    // Initialize graphics and console
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    while (aptMainLoop()) {
        show_menu();
        
        // Scan input
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_A) {
            // Play Blackjack
            printf("\x1b[2J");  // Clear screen
            printf("Starting Blackjack...\n");
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
            
            blackjack_main(argc, argv);
        }
        else if (kDown & KEY_B) {
            // Play Roulette
            printf("\x1b[2J");  // Clear screen
            printf("Starting Roulette...\n");
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
            
            roulette_main(argc, argv);
        }
        else if (kDown & KEY_Y) {
            // Launch Text Editor
            printf("\x1b[2J");  // Clear screen
            printf("Starting Text Editor...\n");
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
            
            text_editor_main();
        }
        else if (kDown & KEY_X) {
            // Exit
            break;
        }

        // Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    // Exit services
    gfxExit();
    return 0;
}