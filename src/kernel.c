#include "kernel.h"
#include "font.h"
#include "framebf.h"
#include "interrupt.h"
#include "mbox.h"
#include "timer.h"
#include "uart.h"
#include "util.h"

#include "game_background.h"
#include "instruction_screen.h"
#include "picture.h"
#include "sprite.h"

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

void main() {
    // Set up serial console
    uart_init();

    // Initialize frame buffer with offset (200, 200)
    framebf_init(200, 200);

    // Draw initial image
    drawImage(image_x, image_y, sample_pic, IMAGE_WIDTH, IMAGE_HEIGHT);

    // Keep track of the previous position of the image
    int prev_image_y = image_y;

    // Main loop
    while (1) {
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
        case 0: // Image display

                    //Display text
                    drawStringCentered(50, "Group 8 - Members:", 0xFFFFFFFF, 2, width);
                    drawStringCentered(150, "Le Thanh Vinh - S3914997", 0xFFFF99CC, 2, width);
                    drawStringCentered(250, "Nguyen Bui Huy Hoang - S3914538", 0xFF99CCFF, 2, width);
                    drawStringCentered(350, "Hoang Nguyen Bao Duy - S3927196", 0xFF99FF99, 2, width);
                    drawStringCentered(450, "Tran Quang - S3976073", 0x00FF9999, 2, width);
            break;
            
        case 1: // Video display

            if (!is_frame_init) {
                framebf_init(0, 0); // Reset offset to 0
                is_frame_init = 1;
            }
            // Do nothing because it's handled by timer interrupt
            break;

        // case 2: // Text display
        //     drawStringCentered(50, "Group 16 - Members:", 0x00000000, 2, width);
        //     drawStringCentered(300, "Tran Dai Phuc - S3924715", 0xFFFF0000, 2, width);
        //     drawStringCentered(400, "Nguyen Hong Anh - S3924711", 0xFFFFFF00, 2, width);
        //     drawStringCentered(500, "Tran Nguyen Quy Khang - S3929513", 0xFF0000FF, 2, width);
        //     drawStringCentered(600, "Tran Minh Truong - S3891643", 0xFF00FF00, 2, width);
        //     break;

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

void drawMap(int rows, int cols, int map[MAX_WIDTH][MAX_HEIGHT], const unsigned long *allArray[allArray_LEN], int screenWidth, int screenHeight) {
    // Calculate the total width and height of the map
    int totalMapWidth = cols * SPRITE_WIDTH;
    int totalMapHeight = rows * SPRITE_HEIGHT;

    // Calculate the offsets to center the map
    int map_offset_x = (screenWidth - totalMapWidth) / 2;
    int map_offset_y = (screenHeight - totalMapHeight) / 2;

    // Iterate over the rows of the map
    for (int i = 0; i < rows; i++) {
        // Iterate over the columns of the map
        for (int j = 0; j < cols; j++) {
            // Calculate the x and y coordinates for the image
            int x = map_offset_x + j * SPRITE_WIDTH;
            int y = map_offset_y + i * SPRITE_HEIGHT;

            // Get the image for the current map element
            const unsigned long *image = allArray[map[i][j]];

            // Draw the image
            drawImage(x, y, image, SPRITE_WIDTH, SPRITE_HEIGHT);
        }
    }
}

void movePlayer(int mapHeight, int mapWidth, int map[MAX_HEIGHT][MAX_WIDTH], int direction) {
    int dx = 0, dy = 0;
    // Determine the direction of movement
    switch (direction) {
    case 'w':
        dy = -1;
        break; // Up
    case 's':
        dy = 1;
        break; // Down
    case 'a':
        dx = -1;
        break; // Left
    case 'd':
        dx = 1;
        break; // Right
    }

    // Get player's current position
    int playerX, playerY;
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            if (map[y][x] == PLAYER) {
                playerX = x;
                playerY = y;
            }
        }
    }

    // Check what's in the new position
    int newX = playerX + dx;
    int newY = playerY + dy;
    int newTile = map[newY][newX];

    // Check if the new position is a wall
    if (newTile == WALL) {
        return; // Can't move into a wall
    }

    if (newTile == WATER) {
        return; // Can't move into a wall
    }

    // Check if the new position is a block
    if (newTile == BLOCK) {
        // Check what's beyond the block
        int newBlockX = newX + dx;
        int newBlockY = newY + dy;
        int newBlockTile = map[newBlockY][newBlockX];

        // Can't push the block if there's a wall or another block behind it
        if (newBlockTile == WALL || newTile == WATER || newBlockTile == BLOCK || newBlockTile == SUCCESS_BLOCK) {
            return;
        }

        // Move the block
        map[newBlockY][newBlockX] = (newBlockTile == END_POINT) ? SUCCESS_BLOCK : BLOCK;
    }

    // Check if the new position is a success block
    if (newTile == SUCCESS_BLOCK) {
        // Check what's beyond the block
        int newBlockX = newX + dx;
        int newBlockY = newY + dy;
        int newBlockTile = map[newBlockY][newBlockX];

        // Can't push the block if there's a wall or another block behind it
        if (newBlockTile == WALL || newTile == WATER || newBlockTile == BLOCK || newBlockTile == SUCCESS_BLOCK) {
            return;
        }

        // Move the block
        map[newBlockY][newBlockX] = (newBlockTile == END_POINT) ? SUCCESS_BLOCK : BLOCK;
        // Set the previous tile to end point
        map[newY][newX] = END_POINT;
    }

    // Move the player
    map[playerY][playerX] = (previousTile == END_POINT) ? END_POINT : EMPTY; // Restore the previous tile only if it was an end point
    // Save the current tile before moving the player
    previousTile = map[newY][newX];
    map[newY][newX] = PLAYER;
}

int isLevelComplete(int mapHeight, int mapWidth, int map[mapHeight][mapWidth]) {
    for (int y = 0; y <= mapHeight + 1; y++) {
        for (int x = 0; x <= mapWidth + 1; x++) {
            // Set the flag for the restarting mode
            if (map[y][x] == END_POINT) {
                flag_RestartText = 1;
            }
            // If there's still a block not on an end point, the level is not complete
            if (map[y][x] == BLOCK) {
                return 0;
            }
        }
    }
    return 1; // If all blocks are on end points, the level is complete
}

void welcome_screen_display() {
    while (1) {
        char c = uart_getc(); // Read user input
        handle_irq();

        // Only process input if a character was received
        if (c != 0) {
            if (c == '\n') { // "Enter" key to switch display
                is_welcome_screen = 0;
                drawRectARGB32(0, 0, 1920, 1080, 0xFF000000, 1); // Clear the screen
                break;
            }
        }
    }
}

void instruction_screen_display() {
    int instruction_x = (width - INSTRUCTION_WIDTH) / 2;
    int instruction_y = (height - INSTRUCTION_HEIGHT) / 2;
    // Display background
    drawImage(instruction_x, instruction_y, instruction_screen, INSTRUCTION_WIDTH, INSTRUCTION_HEIGHT);

    // Display instruction
    drawStringCentered(50, "Instruction!!!", 0xFFFFCCFF, 6, width);
    drawStringCentered(150, "Before you start, please follow this game instruction:", 0xFFFFFFFF, 2, width);
    drawStringCentered(200, "Press 'w' to move Up", 0xFFFFFFFF, 2, width);
    drawStringCentered(250, "Press 's' to move Down", 0xFFFFFFFF, 2, width);
    drawStringCentered(300, "Press 'a' to move Left", 0xFFFFFFFF, 2, width);
    drawStringCentered(350, "Press 'd' to move Right", 0xFFFFFFFF, 2, width);
    drawStringCentered(400, "Press 'r' to restart the level", 0xFFFFFFFF, 2, width);
    drawStringCentered(450, "Press 'h' to see hint", 0xFFFFFFFF, 2, width);
    drawStringCentered(560, "Press Enter to continue", 0xFFFFFFFF, 4, width);

    // "Enter" key to switch display
    while (1) {
        char c = uart_getc(); // Read user input

        // Only process input if a character was received
        if (c != 0) {
            if (c == '\n') { // "Enter" key to switch display
                drawRectARGB32(0, 0, 1920, 1080, 0xFF000000, 1); // Clear the screen
                displayInstructionsinTerminal();                 // Start guiding the user to play the game
                break;
            }
        }
    }
}

// called after all levels are completed
// showing information of team members and acknowledgement
void end_screen_display() {
    // keep track of the horizontal movement of Sprite
    static int x = 0;

    // fill the screen with grass/Empty Sprite
    for (int y = 0; y <= sprite_res * 11; y += sprite_res) {
        for (int x = 0; x <= sprite_res * 16; x += sprite_res) {
            drawImage(x, y, Empty, sprite_res, sprite_res);
        }
    }

    while (1) {
        char c = uart_getc();  // Read user input
        if (text_y == -1200) { // reset y for text
            text_y = 0;
        }
        if (x == sprite_res * 16) { // reset x for Sprite
            x = 0;
        }

        // text is reprinted in the loop with the vertical position changes controlled by 'text_y' variable
        drawStringCentered(text_y + 400, "Thanks for enjoying the game", 0xFB2943, 3, width);
        drawStringCentered(text_y + 650, "Welcome Screen and instruction screen", 0x964B00, 2, width);
        drawStringCentered(text_y + 950, "PRESS ENTER: MAIN MENU", 0xFF, 2, width);

        // decrease each loop
        text_y -= 20;

        // draw the player Sprite
        drawImage(x, 675, Player, sprite_res, sprite_res);

        // update position
        x += sprite_res;

        // delay to make sure that it does not stutter the frames
        wait_msec(500);

        // redraw the background filled with grass/Emtpy again
        for (int y = 0; y <= sprite_res * 11; y += sprite_res) {
            for (int x = 0; x <= sprite_res * 16; x += sprite_res) {
                drawImage(x, y, Wall, sprite_res, sprite_res);
            }
        }

        // fill the previous position of player with grass
        drawImage(x - sprite_res, 675, Empty, sprite_res, sprite_res);

        // draw a Block in front of a Player, since we add o delay so the Player will appear instantly after being deleted
        drawImage(x + sprite_res, 675, Block, sprite_res, sprite_res);

        // Only prcess input if a character was received
        if (c != 0) {
            if (c == '\n') { // "Enter" key to switch display
                is_welcome_screen = 1;

                // reset the text_y variable
                text_y = 200;
                drawRectARGB32(0, 0, 1920, 1080, 0xFF000000, 1); // Clear the screen
                break;
            }
        }
    }
}

// reset all levels for game initialization
void reset_levels() {
    for (int i = 0; i <= NUM_LEVELS - 1; i++) {
        levels[i].width = initialLevels[i].width;
        levels[i].height = initialLevels[i].height;
        for (int j = 0; j < MAX_WIDTH; j++) {
            for (int k = 0; k < MAX_HEIGHT; k++) {
                levels[i].map[j][k] = initialLevels[i].map[j][k];
            }
        }
    }
}

// Display the text to indicate the process of restarting
void displayRestartText() {
    if (flag_RestartText) {
        drawString(width / 2 - 150, height - 50, "Restarting level...", 0xFF00FF00, 2);
        wait_msec(500);
        drawRectARGB32(0, 0, 1920, 1080, 0xFF000000, 1);                       // Clear the screen
        drawImage(0, 0, game_background, BACKGROUND_WIDTH, BACKGROUND_HEIGHT); // Draw the image back again
    }
}

void drawSplitScreen(int rows, int cols, int map[MAX_WIDTH][MAX_HEIGHT]) {
    int instructionsWidth = 40;              // Width for instructions section
    int mapStartCol = instructionsWidth + 2; // Start column for the map

    // Display instructions on the left
    displayInstructionsinTerminal();

    // Display map on the right
    uart_puts("\033[4;82H"); // Position cursor for the map (row 4, col 82)

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            switch (map[i][j]) {
            case EMPTY:
                uart_puts(".");
                break;
            case WALL:
                uart_puts("#");
                break;
            case BLOCK:
                uart_puts("B");
                break;
            case SUCCESS_BLOCK:
                uart_puts("*");
                break;
            case END_POINT:
                uart_puts("E");
                break;
            case PLAYER:
                uart_puts("P");
                break;
            default:
                uart_puts("?");
                break; // For unknown elements
            }
        }
        uart_puts("\033[1E");  // Move cursor down one line
        uart_puts("\033[82G"); // Move cursor back to the starting column for the map
    }
}

void displayInstructionsinTerminal() {
    uart_puts("\033[2J\033[H");
    uart_puts("\n------ Sokoban Instructions ------\n");
    uart_puts("Instruction to move the player using:\n");
    uart_puts("  'w' - go Up\n");
    uart_puts("  's' - go Down\n");
    uart_puts("  'a' - go Left\n");
    uart_puts("  'd' - go Right\n");
    uart_puts("Push the blocks (Square Wooden Box) onto the targets [The Dot Yellow icon(.)]\n");
    uart_puts("Press 'r' to restart the level.\n");
    uart_puts("Press 't' to auto-completed the level\n");
    uart_puts("----------------------------------\n\n");
}
