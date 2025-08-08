/*
 * Simple Text Editor for Nintendo 3DS
 * 
 * This file implements a basic text editor that uses the 3DS's built-in
 * software keyboard for text input and provides basic file operations.
 */

#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TEXT_SIZE 4096
#define MAX_FILENAME 256
#define LINES_PER_PAGE 20
#define CHARS_PER_LINE 50
#define MAX_INPUT_SIZE 512

typedef struct {
    char text[MAX_TEXT_SIZE];
    char filename[MAX_FILENAME];
    int text_length;
    int cursor_pos;
    int scroll_offset;
    int modified;
} TextEditor;

// Function prototypes
void init_text_editor(TextEditor* editor);
void display_text(TextEditor* editor);
void display_menu(TextEditor* editor);
int get_text_input(char* buffer, int max_length, const char* hint);
int get_filename_input(char* filename);
void save_file(TextEditor* editor);
void load_file(TextEditor* editor);
void new_file(TextEditor* editor);
void insert_text(TextEditor* editor, const char* new_text);
void delete_char(TextEditor* editor);
void move_cursor(TextEditor* editor, int direction);
void show_help(void);
void text_editor_main(void);

void init_text_editor(TextEditor* editor) {
    memset(editor->text, 0, MAX_TEXT_SIZE);
    memset(editor->filename, 0, MAX_FILENAME);
    strcpy(editor->filename, "untitled.txt");
    editor->text_length = 0;
    editor->cursor_pos = 0;
    editor->scroll_offset = 0;
    editor->modified = 0;
}

void display_text(TextEditor* editor) {
    printf("\x1b[2J\x1b[1;1H"); // Clear screen and move to top-left
    
    // Header
    printf("Text Editor - %s%s\n", editor->filename, editor->modified ? " *" : "");
    printf("Pos: %d/%d | Lines: ~%d\n", editor->cursor_pos, editor->text_length, 
           (editor->text_length / CHARS_PER_LINE) + 1);
    printf("=====================================\n");
    
    // Display text with simple pagination
    int line = 0;
    int displayed_chars = 0;
    
    for (int i = editor->scroll_offset; i < editor->text_length && line < LINES_PER_PAGE; i++) {
        if (i == editor->cursor_pos) {
            printf("|"); // Cursor indicator
        }
        
        char c = editor->text[i];
        if (c == '\n') {
            printf("\n");
            line++;
            displayed_chars = 0;
        } else if (c >= 32 && c <= 126) { // Printable ASCII
            printf("%c", c);
            displayed_chars++;
            
            // Auto-wrap long lines
            if (displayed_chars >= CHARS_PER_LINE) {
                printf("\n");
                line++;
                displayed_chars = 0;
            }
        }
    }
    
    // Show cursor at end if needed
    if (editor->cursor_pos >= editor->text_length) {
        printf("|");
    }
    
    printf("\n\n");
}

void display_menu(TextEditor* editor) {
    printf("Controls:\n");
    printf("A - Add text at cursor\n");
    printf("B - Delete character\n");
    printf("X - Save file\n");
    printf("Y - Load file\n");
    printf("L - New file\n");
    printf("R - Show help\n");
    printf("UP/DOWN - Move cursor\n");
    printf("LEFT/RIGHT - Scroll view\n");
    printf("START - Exit to main menu\n");
}

int get_text_input(char* buffer, int max_length, const char* hint) {
    SwkbdState swkbd;
    SwkbdButton button;
    
    swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, max_length);
    swkbdSetHintText(&swkbd, hint);
    swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    
    button = swkbdInputText(&swkbd, buffer, max_length);
    
    return (button == SWKBD_BUTTON_CONFIRM);
}

int get_filename_input(char* filename) {
    SwkbdState swkbd;
    SwkbdButton button;
    
    swkbdInit(&swkbd, SWKBD_TYPE_WESTERN, 2, MAX_FILENAME - 1);
    swkbdSetHintText(&swkbd, "Enter filename (e.g., document.txt)");
    swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    
    button = swkbdInputText(&swkbd, filename, MAX_FILENAME - 1);
    
    return (button == SWKBD_BUTTON_CONFIRM);
}

void save_file(TextEditor* editor) {
    char temp_filename[MAX_FILENAME];
    strcpy(temp_filename, editor->filename);
    
    if (get_filename_input(temp_filename)) {
        strcpy(editor->filename, temp_filename);
        
        FILE* file = fopen(editor->filename, "w");
        if (file) {
            fwrite(editor->text, 1, editor->text_length, file);
            fclose(file);
            editor->modified = 0;
            printf("File saved successfully!\n");
        } else {
            printf("Error: Could not save file!\n");
        }
    }
    
    // Wait for user acknowledgment
    printf("Press A to continue...\n");
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_A) break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
}

void load_file(TextEditor* editor) {
    char temp_filename[MAX_FILENAME];
    memset(temp_filename, 0, MAX_FILENAME);
    
    if (get_filename_input(temp_filename)) {
        FILE* file = fopen(temp_filename, "r");
        if (file) {
            // Get file size
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            if (file_size < MAX_TEXT_SIZE) {
                memset(editor->text, 0, MAX_TEXT_SIZE);
                editor->text_length = fread(editor->text, 1, file_size, file);
                strcpy(editor->filename, temp_filename);
                editor->cursor_pos = 0;
                editor->scroll_offset = 0;
                editor->modified = 0;
                printf("File loaded successfully!\n");
            } else {
                printf("Error: File too large (max %d chars)!\n", MAX_TEXT_SIZE);
            }
            fclose(file);
        } else {
            printf("Error: Could not open file!\n");
        }
    }
    
    // Wait for user acknowledgment
    printf("Press A to continue...\n");
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_A) break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
}

void new_file(TextEditor* editor) {
    if (editor->modified) {
        printf("Current file has unsaved changes!\n");
        printf("A - Continue (lose changes), B - Cancel\n");
        
        while (aptMainLoop()) {
            hidScanInput();
            u32 kDown = hidKeysDown();
            
            if (kDown & KEY_A) {
                break; // Continue with new file
            } else if (kDown & KEY_B) {
                return; // Cancel
            }
            
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
        }
    }
    
    init_text_editor(editor);
    printf("New file created!\n");
    
    // Wait for user acknowledgment
    printf("Press A to continue...\n");
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_A) break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
}

void insert_text(TextEditor* editor, const char* new_text) {
    int new_text_len = strlen(new_text);
    
    // Check if we have space
    if (editor->text_length + new_text_len >= MAX_TEXT_SIZE) {
        printf("Error: Text too long!\n");
        return;
    }
    
    // Move existing text to make room
    memmove(&editor->text[editor->cursor_pos + new_text_len],
            &editor->text[editor->cursor_pos],
            editor->text_length - editor->cursor_pos);
    
    // Insert new text
    memcpy(&editor->text[editor->cursor_pos], new_text, new_text_len);
    
    // Update counters
    editor->text_length += new_text_len;
    editor->cursor_pos += new_text_len;
    editor->modified = 1;
}

void delete_char(TextEditor* editor) {
    if (editor->cursor_pos > 0) {
        // Move text backward to overwrite deleted character
        memmove(&editor->text[editor->cursor_pos - 1],
                &editor->text[editor->cursor_pos],
                editor->text_length - editor->cursor_pos);
        
        editor->cursor_pos--;
        editor->text_length--;
        editor->text[editor->text_length] = '\0';
        editor->modified = 1;
    }
}

void move_cursor(TextEditor* editor, int direction) {
    if (direction > 0 && editor->cursor_pos < editor->text_length) {
        editor->cursor_pos++;
    } else if (direction < 0 && editor->cursor_pos > 0) {
        editor->cursor_pos--;
    }
    
    // Adjust scroll if cursor moves out of view
    if (editor->cursor_pos < editor->scroll_offset) {
        editor->scroll_offset = editor->cursor_pos;
    } else if (editor->cursor_pos >= editor->scroll_offset + (LINES_PER_PAGE * CHARS_PER_LINE)) {
        editor->scroll_offset = editor->cursor_pos - (LINES_PER_PAGE * CHARS_PER_LINE) + 1;
    }
}

void show_help(void) {
    printf("\x1b[2J\x1b[1;1H"); // Clear screen
    printf("Text Editor Help\n");
    printf("================\n\n");
    printf("Controls:\n");
    printf("A Button - Open software keyboard to add text\n");
    printf("B Button - Delete character before cursor\n");
    printf("X Button - Save current file\n");
    printf("Y Button - Load a file\n");
    printf("L Button - Create new file\n");
    printf("R Button - Show this help screen\n");
    printf("D-Pad UP - Move cursor up/backward\n");
    printf("D-Pad DOWN - Move cursor down/forward\n");
    printf("D-Pad LEFT - Scroll view left\n");
    printf("D-Pad RIGHT - Scroll view right\n");
    printf("START - Exit to main menu\n\n");
    
    printf("Features:\n");
    printf("- Save/Load text files to SD card\n");
    printf("- Basic cursor navigation\n");
    printf("- File modification indicator (*)\n");
    printf("- Auto-wrapping for long lines\n");
    printf("- Up to %d characters per file\n\n", MAX_TEXT_SIZE);
    
    printf("Press A to return to editor...\n");
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_A) break;
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
}

void text_editor_main(void) {
    TextEditor editor;
    char input_buffer[MAX_INPUT_SIZE];
    
    init_text_editor(&editor);
    
    printf("Simple Text Editor\n");
    printf("Press A to continue...\n");
    
    // Wait for initial input
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_A) break;
        if (kDown & KEY_START) return; // Exit immediately
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    // Main editor loop
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        // Clear screen and display current state
        display_text(&editor);
        display_menu(&editor);
        
        // Handle input
        if (kDown & KEY_A) {
            // Add text using software keyboard
            memset(input_buffer, 0, MAX_INPUT_SIZE);
            if (get_text_input(input_buffer, MAX_INPUT_SIZE - 1, "Enter text to add:")) {
                insert_text(&editor, input_buffer);
            }
        }
        else if (kDown & KEY_B) {
            // Delete character
            delete_char(&editor);
        }
        else if (kDown & KEY_X) {
            // Save file
            save_file(&editor);
        }
        else if (kDown & KEY_Y) {
            // Load file
            load_file(&editor);
        }
        else if (kDown & KEY_L) {
            // New file
            new_file(&editor);
        }
        else if (kDown & KEY_R) {
            // Show help
            show_help();
        }
        else if (kDown & KEY_DUP) {
            // Move cursor backward
            move_cursor(&editor, -1);
        }
        else if (kDown & KEY_DDOWN) {
            // Move cursor forward
            move_cursor(&editor, 1);
        }
        else if (kDown & KEY_DLEFT) {
            // Scroll view left
            if (editor.scroll_offset > 0) {
                editor.scroll_offset--;
            }
        }
        else if (kDown & KEY_DRIGHT) {
            // Scroll view right
            if (editor.scroll_offset < editor.text_length) {
                editor.scroll_offset++;
            }
        }
        else if (kDown & KEY_START) {
            // Exit to main menu
            if (editor.modified) {
                printf("File has unsaved changes!\n");
                printf("A - Save and exit, B - Exit without saving, X - Cancel\n");
                
                while (aptMainLoop()) {
                    hidScanInput();
                    u32 kDown2 = hidKeysDown();
                    
                    if (kDown2 & KEY_A) {
                        save_file(&editor);
                        return;
                    } else if (kDown2 & KEY_B) {
                        return;
                    } else if (kDown2 & KEY_X) {
                        break; // Cancel exit
                    }
                    
                    gfxFlushBuffers();
                    gfxSwapBuffers();
                    gspWaitForVBlank();
                }
            } else {
                return; // Exit without prompting
            }
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
}