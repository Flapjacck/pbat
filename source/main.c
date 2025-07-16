#include <3ds.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    // Initialize graphics
    gfxInitDefault();
    
    // Initialize console for top screen
    consoleInit(GFX_TOP, NULL);
    
    printf("Hello World\n");
    
    
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
        
        // Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        
        // Wait for VBlank
        gspWaitForVBlank();
    }
    
    // Exit graphics
    gfxExit();
    return 0;
}
