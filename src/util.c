#include "util.h"
int min(int a, int b) {
    return a < b ? a : b;
}

int max(int a, int b) {
    return a > b ? a : b;
}

int strlen(const char *str) {
    int length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

void wait_msec(unsigned int msVal) {
    register unsigned long f, t, r, expiredTime; // 64 bits

    // Get the current counter frequency (Hz), 1Hz = 1 pulses/second
    asm volatile("mrs %0, cntfrq_el0" : "=r"(f));

    // Read the current counter value
    asm volatile("mrs %0, cntpct_el0" : "=r"(t));

    // Calculate expire value for counter
    /* Note: both expiredTime and counter value t are 64 bits,
    thus, it will still be correct when the counter is overflow */
    expiredTime = t + f * msVal / 1000;

    do {
        asm volatile("mrs %0, cntpct_el0" : "=r"(r));
    } while (r < expiredTime);
}

void processUserInputinTerminal(char c) {
    if (c != '\r') { // '\r' is the carriage return character (Enter key)
        // Position cursor below instructions
        uart_puts("\033[12;1H"); // Assuming instructions take up 11 lines

        // Clear the current line in the terminal
        uart_puts("\033[2K\r"); 

        // Print a message indicating the key pressed
        uart_puts("Key pressed: ");
        uart_sendc(c);
        uart_puts("\n");

        // Annouce to the user if they restart the current level
        if (c == 'r')   {
            uart_puts("Successfully restart the current level from the beginning");
        }
    }
}