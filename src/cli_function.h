#ifndef CLI_FUNCTION_H
#define CLI_FUNCTION_H

// Character defines
#define EMPTY_CHAR 32
#define BACKSPACE 8

// Define constants
#define MAX_CMD_SIZE 100
#define HISTORY_STORAGE 20

// Function prototypes for CLI operations

// Function for auto completion
void handle_auto_completion(char* cli_buffer, int index);

// Function to navigate command history
void navigate_history(int direction, char* cli_buffer, int* index, char cli_buffer_history[HISTORY_STORAGE][MAX_CMD_SIZE], int* cli_buffer_history_index);

// Function for backspace
void backspace(char* cli_buffer, int* index);

// Function to clear the CLI line
void clear_cli_line(char* cli_buffer, int* index);

// Function to remove leading and trailing whitespaces
void remove_whitespace(char *str);

// Function to execute commands
void execute_command(char* cli_buffer);

#endif // CLI_FUNCTION_H
