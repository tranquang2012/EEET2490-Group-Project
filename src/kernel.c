#include "uart.h"
#include "function.h"
#include "cli_function.h"
#include "mbox.h"
#include "string.h"
#include "kernel.h"
#include "util.h"
#include "timer.h"
#include "interrupt.h"
#include "framebf.h"

#include "game_background.h"
#include "instruction_screen.h"
#include "picture.h"
#include "sprite.h"
#include "welcome_screen.h"
#include "video.h"

#define MAX_CMD_SIZE 100
#define HISTORY_STORAGE 20

// Define global variables
static char cli_buffer[MAX_CMD_SIZE];
static int index = 0;
static char cli_buffer_history[HISTORY_STORAGE][MAX_CMD_SIZE] = {
    "", "", "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "", "", ""
};
static int cli_buffer_history_index = 0;
static int history_next = 1;

const char* welcome_message =
    "8888888888 8888888888 8888888888 88888888888 .d8888b.      d8888   .d8888b.   .d8888b.  \n"
    "888        888        888            888    d88P  Y88b    d8P888  d88P  Y88b d88P  Y88b \n"
    "888        888        888            888           888   d8P 888  888    888 888    888 \n"
    "8888888    8888888    8888888        888         .d88P  d8P  888  Y88b. d888 888    888 \n"
    "888        888        888            888     .od888P\"  d88   888   \"Y888P888 888    888 \n"
    "888        888        888            888    d88P\"      8888888888        888 888    888 \n"
    "888        888        888            888    888\"             888  Y88b  d88P Y88b  d88P \n"
    "8888888888 8888888888 8888888888     888    888888888        888   \"Y8888P\"   \"Y8888P\"  \n"
    "                                                                                       \n"
    "888888b.         d8888 8888888b.  8888888888     .d88888b.   .d8888b.                   \n"
    "888  \"88b       d88888 888   Y88b 888           d88P\" \"Y88b d88P  Y88b                  \n"
    "888  .88P      d88P888 888    888 888           888     888 Y88b.                       \n"
    "8888888K.     d88P 888 888   d88P 8888888       888     888  \"Y888b.                    \n"
    "888  \"Y88b   d88P  888 8888888P\"  888           888     888     \"Y88b.                  \n"
    "888    888  d88P   888 888 T88b   888           888     888       \"888                  \n"
    "888   d88P d8888888888 888  T88b  888           Y88b. .d88P Y88b  d88P                  \n"
    "8888888P\" d88P     888 888   T88b 8888888888     \"Y88888P\"   \"Y8888P\"                   \n"
    "\n\n"
    "Developed by <TRAN QUANG> - <S3976073>\n"
    "             <HOANG NGUYEN BAO DUY> - <S3927196>\n"
    "             <LE THANH VINH> - <S3914997>\n"
    "             <NGUYEN BUI HUY HOANG> - <S3914538>\n"
    "Type help for more info\n";

extern unsigned int width;  // Width of the screen
extern unsigned int height; // Height of the screen

// Position of the top-left corner of the image
int image_x = 0;
int image_y = 0;

// Vertical position of texts
int text_y = 200;

// State of the program
int state = 0;

// Variable to check if the welcome screen is displayed
int is_welcome_screen = 1;

// Variable to check if the frame is initialize for video display
int is_frame_init = 0;

// Attributes for game
int previousTile = EMPTY; // To keep track of the player's previous tile
int currentLevel = 0; // Track current level
int gameWon = 0;      // Check if game is complete
int actualLevel = 0;
int flag_RestartText = 1; // Make a flag to prevent and control the Text Remover. 1 = Can be removed, 2 = Not removed
int sprite_res = 64;  // Resolution of game sprites

void display_welcome_message() {
    uart_puts(welcome_message);
}

void cli() {
    char c = uart_getc();

    if (c == '\t') {
        // Handle TAB key for auto-completion
        TAB(cli_buffer, &index); // Assuming TAB() function is defined elsewhere
    } else if (c == '_' || c == '+') {
        // Handle history navigation
        if (c == '_') {
            // Display previous command from history
            clear_cli_line(cli_buffer, &index);

            if ((cli_buffer_history_index < HISTORY_STORAGE - 1) && 
                (comp_str(cli_buffer_history[cli_buffer_history_index + 1], "") != 0)) {
                cli_buffer_history_index++; // Move to the next index
                record_buffer(cli_buffer_history[cli_buffer_history_index], cli_buffer, &index);
                uart_puts(cli_buffer);
            } else if (cli_buffer_history_index < HISTORY_STORAGE && history_next) {
                cli_buffer_history_index++; // Move to the next index
                history_next = 0;
            }
        } else if (c == '+') {
            // Clear line
            clear_cli_line(cli_buffer, &index);
            history_next = 1;

            // Get the latest recorded buffer cli
            if (cli_buffer_history_index > -1) {
                cli_buffer_history_index--;
                record_buffer(cli_buffer_history[cli_buffer_history_index], cli_buffer, &index);
                uart_puts(cli_buffer);
            } else if (cli_buffer_history_index == -1) {
                history_next = -1;
            }
        }
    } else if (c == BACKSPACE) {
        // Handle backspace key
        if (index > 0) {
            index--;
            cli_buffer[index] = '\0'; // Remove last character from buffer
            uart_delete(); // Assuming uart_delete() updates terminal display
        }
    } else if (c != '\n') {
        // Handle regular input
        if (index < MAX_CMD_SIZE - 1) {
            uart_sendc(c); // Echo character
            cli_buffer[index++] = c; // Update buffer
        }
    } else if (c == '\n') {
        // Handle end of command
        uart_sendc(c); // Echo newline

        cli_buffer[index] = '\0'; // Null-terminate the buffer

        // Save command to history
        if (cli_buffer_history_index < HISTORY_STORAGE - 1) {
            cli_buffer_history_index++;
        } else {
            // Shift history if storage is full
            for (int i = 1; i < HISTORY_STORAGE; i++) {
                copy_cli_buffer(cli_buffer_history[i], cli_buffer_history[i - 1]);
            }
            cli_buffer_history_index = HISTORY_STORAGE - 1;
        }
        record_buffer(cli_buffer, cli_buffer_history[cli_buffer_history_index], &index);

        // Execute command
        execute_command(cli_buffer);

        // Reset buffer
        reset_arr(cli_buffer);
        index = 0; // Reset index

        // Re-print prompt
        uart_puts("\nMyOS>: ");
    }
}

void main() {
    // Initialize UART and display the welcome message
    uart_init();
    display_welcome_message();
    uart_puts("\nMyOS>: ");
    
    // Initialize frame buffer with offset (200, 200)
    framebf_init(200, 200);
    
    // Draw initial image
    drawImage(image_x, image_y, sample_pic, IMAGE_WIDTH, IMAGE_HEIGHT);
    
    // Keep track of the previous position of the image
    int prev_image_y = image_y;

    // Run CLI and game functionalities concurrently
    while (1) {
        // Run CLI to handle commands in parallel
        cli();

        handle_irq();
        char c = uart_getc(); // Read user input

        // Only process input if a character was received
        if (c != 0) {
            if (c == ' ') { // "Space" key to switch display
                // Set up/reset interrupt and timer
                enable_interrupt_controller();
                timer_init();
                state += 1;
                drawRectARGB32(0, 0, 1920, 1080, 0xFF000000, 1); // Clear the screen
            }
        }

        switch (state) {
        case 1: // Video display

            if (!is_frame_init) {
                framebf_init(0, 0); // Reset offset to 0
                is_frame_init = 1;
            }
            // Do nothing because it's handled by timer interrupt
            break;

        case 2:  // Game
            // Create a copy of the map
            for (int i = 0; i < NUM_LEVELS; i++) {
                initialLevels[i].width = levels[i].width;
                initialLevels[i].height = levels[i].height;
                for (int j = 0; j < MAX_WIDTH; j++) {
                    for (int k = 0; k < MAX_HEIGHT; k++) {
                        initialLevels[i].map[j][k] = levels[i].map[j][k];
                    }
                }
            }

            // Welcome screen display
            welcome_screen_display();

            // Instruction screen display
            instruction_screen_display();

            // Display game background
            drawImage(0, 0, game_background, BACKGROUND_WIDTH, BACKGROUND_HEIGHT);

            // Main program
            while (1) {
                char c = uart_getc(); // Read user input

                // Check if all level is completed
                if (gameWon) {
                    // call the end screen function
                    end_screen_display();

                    // end_screen is done, intitialize the whole game
                    gameWon = 0;
                    is_welcome_screen = 1;
                    previousTile = EMPTY;
                    currentLevel = 0;
                    gameWon = 0;
                    actualLevel = 0;

                    // and also all the levels
                    reset_levels();
                    enable_interrupt_controller();
                    timer_init();
                    welcome_screen_display();
                    drawImage(0, 0, game_background, BACKGROUND_WIDTH, BACKGROUND_HEIGHT);
                    continue;
                }

                // Restart the game if 'r' key is pressed
                if (c == 'r') {
                    displayRestartText();
                    // Guide the user how to play this game
                    drawSplitScreen(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map);
                    // Continuously display the pressed key
                    processUserInputinTerminal(c);
                    levels[currentLevel].width = initialLevels[currentLevel].width;
                    levels[currentLevel].height = initialLevels[currentLevel].height;
                    for (int j = 0; j < MAX_WIDTH; j++) {
                        for (int k = 0; k < MAX_HEIGHT; k++) {
                            levels[currentLevel].map[j][k] = initialLevels[currentLevel].map[j][k];
                        }
                    }

                    // Reset the previous tile variables
                    previousTile = EMPTY;
                }

                // Autocomplete level if 't' key is pressed
                else if (c == 't') {
                    // Restart the level first
                    displayRestartText();
                    drawSplitScreen(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map);
                    processUserInputinTerminal(c);
                    levels[currentLevel].width = initialLevels[currentLevel].width;
                    levels[currentLevel].height = initialLevels[currentLevel].height;
                    for (int j = 0; j < MAX_WIDTH; j++) {
                        for (int k = 0; k < MAX_HEIGHT; k++) {
                            levels[currentLevel].map[j][k] = initialLevels[currentLevel].map[j][k];
                        }
                    }
                    previousTile = EMPTY;

                    wait_msec(500);

                    // Start autocomplete level
                    char *solution = levels[currentLevel].solution;
                    for (int i = 0; solution[i] != '\0'; i++) {
                        char move = solution[i];
                        movePlayer(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map, move);
                        drawMap(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map, allArray, width, height);
                        wait_msec(250);
                    }
                }

                // Movement keys
                else if (c == 'w' || c == 'a' || c == 's' || c == 'd') {
                    movePlayer(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map, c);
                    // Guide the user how to play this game
                    drawSplitScreen(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map);
                    // Continuously display the pressed key
                    processUserInputinTerminal(c);
                }

                // After every action, redraw the map
                drawMap(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map, allArray, width, height);

                // Check if level is complete
                if (isLevelComplete(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map)) {
                    // set the currentLevel to 3 to run all Level completed code
                    if (actualLevel == NUM_LEVELS - 1) {
                        currentLevel = NUM_LEVELS;
                    }

                    // If there are still a level after
                    if (currentLevel < NUM_LEVELS) {
                        // Player has finished transtion level, move to the next actual Level
                        if (currentLevel == TRANSITION_LEVEL) {
                            currentLevel = actualLevel;
                            // Display game background
                            drawRectARGB32(0, 0, 1280, 720, 0, 1); // Create a black screen
                            drawImage(0, 0, game_background, BACKGROUND_WIDTH, BACKGROUND_HEIGHT);

                            // reset level 3 for the next level transtion
                            levels[TRANSITION_LEVEL].width = initialLevels[TRANSITION_LEVEL].width;
                            levels[TRANSITION_LEVEL].height = initialLevels[TRANSITION_LEVEL].height;
                            for (int j = 0; j < MAX_WIDTH; j++) {
                                for (int k = 0; k < MAX_HEIGHT; k++) {
                                    levels[TRANSITION_LEVEL].map[j][k] = initialLevels[TRANSITION_LEVEL].map[j][k];
                                }
                            }

                            // Reset the previous tile variables
                            previousTile = EMPTY;
                            // draw the map for the transition level an continue
                            drawMap(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map, allArray, width, height);
                            continue;

                            // the game is in the actual Level, display a transition map
                        }
                        if (currentLevel != TRANSITION_LEVEL) {
                            flag_RestartText = 0;            // Deactivate the removal function
                            currentLevel = TRANSITION_LEVEL; // display the transtition level
                            actualLevel++;                   // when the transtition is completed, this level will be displayed

                            // fill the background with Block
                            for (int y = 0; y <= sprite_res * 10; y += sprite_res) {
                                for (int x = 0; x <= sprite_res * 16; x += sprite_res) {
                                    drawImage(x, y, Block, sprite_res, sprite_res);
                                }
                            }

                            // if the last level completed display you won
                            if (actualLevel == NUM_LEVELS - 1) {
                                drawStringCentered(50, "!!! CONGRATS !!!", 0xFF00FF00, 5, width);
                                drawStringCentered(150, "YOU WON", 0xFF00FF00, 2, width);
                            } else { // display level completed
                                drawStringCentered(50, "!!! CONGRATS !!!", 0xFF00FF00, 5, width);
                                drawStringCentered(150, "LEVEL COMPLETED", 0xFF00FF00, 2, width);
                            }
                            drawMap(levels[currentLevel].height, levels[currentLevel].width, levels[currentLevel].map, allArray, width, height);
                        }
                    }
                    // All levels completed
                    else {
                        wait_msec(1500);
                        drawRectARGB32(0, 0, 1280, 720, 0, 1); // Create a black screen
                        gameWon = 1;
                    }
                }
            }
            break;
        }
    }
}
