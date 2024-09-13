#include "uart.h"
#include "function.h"
#include "cli_function.h"
#include "mbox.h"
#include "string.h"

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
    // Initialize system
    
    uart_init();
    display_welcome_message();
    uart_puts("\nMyOS>: ");
    
    // Run CLI
    while (1) {
        cli();
    }
}
