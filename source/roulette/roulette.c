#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int roulette_main(int argc, char* argv[])
{
    // Graphics already initialized by main menu
    consoleInit(GFX_TOP, NULL);
    
    printf("\x1b[2J");  // Clear screen
    printf("\x1b[1;1H"); // Move cursor to top-left
    
    printf("========================================\n");
    printf("             ROULETTE                   \n");
    printf("========================================\n");
    printf("\n");
    printf("Welcome to Roulette!\n");
    printf("\n");
    printf("This game is currently under development.\n");
    printf("Coming soon: Full roulette gameplay with\n");
    printf("betting on numbers, colors, and ranges!\n");
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
    
    return 0;
}