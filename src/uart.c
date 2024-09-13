// -----------------------------------uart.c -------------------------------------
#include "uart.h"
#include "gpio.h"
/**
 * Set baud rate and characteristics (115200 8N1) and map to GPIO
 */
void uart_init() {
    unsigned int r;
    AUX_ENABLE |= 1;   // Enable mini UART (UART1)
    AUX_MU_CNTL = 0;   // Disable transmitter and receiver
    AUX_MU_LCR = 3;    // 8 bits, 1 stop bit, no parity
    AUX_MU_MCR = 0;    // Clear RTS (request to send)
    AUX_MU_IER = 0;    // Disable interrupts
    AUX_MU_IIR = 0xc6; // Enable and clear FIFOs
    AUX_MU_BAUD = 270; // Configure 115200 baud

    // GPIO pin configuration
    r = GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // Clear bits 17-12
    r |= (2 << 12) | (2 << 15);    // Set ALT5 for TXD1/RXD1
    GPFSEL1 = r;

    // Enable GPIO pins
    GPPUD = 0; // No pull-up/down
    r = 150;
    while (r--) { asm volatile("nop"); }
    GPPUDCLK0 = (1 << 14) | (1 << 15); // Enable clock for GPIO 14, 15
    r = 150;
    while (r--) { asm volatile("nop"); }
    GPPUDCLK0 = 0; // Flush GPIO setup

    AUX_MU_CNTL = 3; // Enable transmitter and receiver

    uart_puts("Initialized UART. LCR value: ");
    uart_hex(AUX_MU_LCR);
    uart_puts("\n");
}


/**
 * Send a character
 */
void uart_sendc(char c)
{
    // wait until transmitter is empty
    do
    {
        asm volatile("nop");
    } while (!(AUX_MU_LSR & 0x20));
    // write the character to the buffer
    AUX_MU_IO = c;
}
/**
 * Receive a character
 */
char uart_getc()
{
    char c;
    // wait until data is ready (one symbol)
    do
    {
        asm volatile("nop");
    } while (!(AUX_MU_LSR & 0x01));
    // read it and return
    c = (unsigned char)(AUX_MU_IO);
    // convert carriage return to newline character
    return (c == '\r' ? '\n' : c);
}
/**
 * Display a string
 */
void uart_puts(char *s)
{
    while (*s)
    {
        // convert newline to carriage return + newline
        if (*s == '\n')
            uart_sendc('\r');
        uart_sendc(*s++);
    }
}

/**
 * Display a value in hexadecimal format
 */
void uart_hex(unsigned int num)
{
    uart_puts("0x");
    for (int pos = 28; pos >= 0; pos = pos - 4)
    {
        // Get highest 4-bit nibble
        char digit = (num >> pos) & 0xF;
        /* Convert to ASCII code */
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        digit += (digit > 9) ? (-10 + 'A') : '0';
        uart_sendc(digit);
    }
}
/**
 * Display a value in decimal format
 */
void uart_dec(int num)
{
    // A string to store the digit characters
    char str[33] = "";
    // Calculate the number of digits
    int len = 1;
    int temp = num;
    while (temp >= 10)
    {
        len++;
        temp = temp / 10;
    }
    // Store into the string and print out
    for (int i = 0; i < len; i++)
    {
        int digit = num % 10; // get last digit
        num = num / 10;       // remove last digit from the number
        str[len - (i + 1)] = digit + '0';
    }
    str[len] = '\0';
    uart_puts(str);
}

void uart_delete() {
    // Move the cursor back one position
    uart_puts("\b");
    // Print a space to overwrite the character
    uart_puts(" ");
    // Move the cursor back again
    uart_puts("\b");
}

/**
 * Get a character with if
 */
char uart_get_char()
{
    char c;
    if (!(AUX_MU_LSR & 0x01))
        asm volatile("nop");

    /* read it and return */
    c = (char)(AUX_MU_IO);

    /* convert carriage return to newline */
    return (c == '\r' ? '\n' : c);
}

/**
 * Receive a character
 */
char uart_getc_game() {
    char c;
	 /* wait until data is ready (one symbol) */
	 int i = 0;
	 int total_time = 1000000;
	 do {
		 asm volatile("nop");
		 i++;
		 if(i == total_time){
			 return '\0';
		 }
	 } while ( !(AUX_MU_LSR & 0x01) ); // - bit 1 FIFO hold at least 1 symbol
	 /* read it and return */
	 c = (char)(AUX_MU_IO); // - read from FIFO as char
	 /* convert carriage return to newline */
	 return (c == '\r' ? '\n' : c); // - check for carriage return if yes change it to Enter

}

/* Function to wait for some msec: the program will stop there */
void wait_msec(unsigned int n)
{
    register unsigned long f, t, r, expiredTime;
    // Get the current counter frequency (Hz)
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    
    // Read the current counter value
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    
    // Calculate expire value for counter
    expiredTime = t + ( (f/1000)*n )/1000;
    do {
     asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
    } while(r < expiredTime);
}