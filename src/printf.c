#include "printf.h"
#include "function.h"
#include "cli_function.h"
#include "uart.h"

#define MAX_PRINT_SIZE 256

// Function to reverse string
void reverse(char* str, int length)
{
    int start = 0;
    int end = length - 1;
    int temp;

    while (start < end) {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;

        start++;
        end--;
    }
}

double pow(double base, double power)  
{  
    // Initial value
    double result = 1;
    long int exp = power;

    // Check if exponent is negative
    if (exp < 0) exp = exp *  (-1);
   
    while (exp){
        if (exp % 2){
            result *= base;
            exp -= 1;
        }
        else {
            base *= base;
            exp /= 2;
        }
    }

    // Check if input power < 0
    if (power < 0) result = (double)(1.0) / (double)result;

    return result;
}

int count_decimal_places(double decimal){
    double num = decimal;
    int dec_count = 0; // Count of Decimal places

    // loop runs until num/(int)num is 1
    while (num / (int)num != 1) {
        dec_count += 1; // Adds 1 to count
        num *= 10;  // num is multipled by 10
    }

    return dec_count;
}

int itoa(int x, char str[], int d){
    int i = 0;

    // Process individual digits
    while (x){
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits is required more, add 0s at the beginning
    while (i < d)
        str[i++] = '\0';

    reverse(str,i);
    str[i] = '\0';
    return i;
}



void ftoa(double n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
 
    // Extract floating part
    double fpart = n - (double)ipart;
 
    // convert integer part to string
    int i = itoa(ipart, res, 0);

    if (afterpoint == -1) afterpoint = count_decimal_places(n);
 
    // check for display option after point
    if (afterpoint != 0) {
        res[i] = '.'; // add dot
 
        fpart = fpart * pow(10, afterpoint);
        fpart += 0.5;   // Round up
 
        itoa((int)fpart, res + i + 1, afterpoint);
    }
}

// Function to seperate the sub-specifier syntax
void argument_check_sub_specfier(char* argument_string, int* width, int* precision, int* flag_zero, int* flag_zero_trig){
    int flag_dot = 0;           // flag for character '.'
    *flag_zero = 0;             // flag 0 sub specifier
    *flag_zero_trig = 0;        // flag trigger the decimal read

    // Check zero flag
    if (*argument_string == '0'){
        *flag_zero = 1;
    }
   
    // Check precision syntax
    if (*argument_string == '.'){
        flag_dot = 1;
        argument_string++;
        if (*argument_string == 'f'){
            *flag_zero_trig = 1;    // enable the flag_zero_trig flag and read the int part
        }
        *precision = *argument_string - '0';
    }

    // Check first character of the argument pointer is 0
    if(flag_dot == 0){
        if (*flag_zero == 0){
            *width = *argument_string - '0';    // Get the width from pointer
        }
        else{
            argument_string++;
            *width = *argument_string - '0';    // Get the width from pointer
        }
    }

    argument_string++;

    // Sweep through the argument specifier to check their width and precision
    // while ((*description != 'd') && (*description != 'f') && (*description != 'x') && (*description != 'o')){
    while ((*argument_string != 'd') && (*argument_string != 'f') && (*argument_string != 'x')){
        if (*argument_string == '.'){
            flag_dot = 1;
            argument_string++;

            // Get the precision value
            *precision = *argument_string - '0';
            argument_string++;
            continue;
        }

        // Update the width and precision from the argument
        if(flag_dot == 0){
            *width = (*width * 10) + (*argument_string - '0');
        }
        else{
            *precision = (*precision * 10) + (*argument_string - '0');
        }
        argument_string++;
    }
}

// %d %c %s %f %x %%, 0 flag, width, and precision
void printf(char *string,...) {

    va_list ap;
    va_start(ap, string);

    char cli_buffer[MAX_PRINT_SIZE];
    int cli_buffer_index = 0;

    int width = 0;
    int precision = 0;
   
    int flag_0 = 0;
    int flag_zero_trig = 0;
   
    char argument_string[MAX_PRINT_SIZE];
    int digit_left = 0;

    char temp_buffer[MAX_PRINT_SIZE];

    while(1) {
        if (*string == 0)
            break;
   
        if (*string == '%') {
            string++;
            // Check width and precision
            if ((*string > 47 && *string < 58) || *string == '.'){
                int wp_index = 0;
                while((*string != 'd') && (*string != 'f') && (*string != 'x')){
                    argument_string[wp_index] = *string;
                    string++;
                    wp_index++;
                }

                argument_string[wp_index] = *string;

                // Sweep through all the specifier of the argument
                if (argument_string[wp_index] == 'd' || argument_string[wp_index] == 'f' || argument_string[wp_index] == 'x'){
                    argument_check_sub_specfier(argument_string, &width, &precision, &flag_0, &flag_zero_trig);
                }
            }
            // Specifier digit - %d
            if (*string == 'd') {
                string++;
                int x = va_arg(ap, int);
                int temp_index = MAX_PRINT_SIZE - 1;
                int flag_negative = 0;

                // Get the total length of the argument
                // int argument_string_length = itoa(x, temp_buffer, precision-1);
               
                // Clear printf argument
                reset_arr(argument_string);

                // Convert negative num to positive num
                // Get the flag for negative sign and update the width by 1
                if(x < 0){
                    x *= (-1);
                    flag_negative = 1;
                    width = width + 1;
                    // width = width - 1;
                }

                // Store each digit into temp buffer
                while(x != 0){
                    temp_buffer[temp_index] = (x % 10) + '0';
                    temp_index--;
                    x /= 10;
                }

                // Check width by digit
                digit_left = width - (MAX_PRINT_SIZE - 1 - temp_index);

                if (digit_left > 0){
                    for (int i = 0; i < digit_left; i++){
                        if (flag_0 == 1)
                        {
                            temp_buffer[temp_index] = '0';
                            temp_index--;
                        }
                        else
                        {
                            if (flag_negative == 1) {
                                temp_buffer[temp_index] = '-';
                                temp_index--;
                                flag_negative = 0;
                            }
                            temp_buffer[temp_index] = ' ';
                            temp_index--;
                        }
                    }
                }
               
                // Add '-' if negative decimal
                // if (flag_negative == -1){
                //  argument_string_length++;
                //  cli_buffer[cli_buffer_index++] = '-';
                // }
                if (flag_negative == 1) {
                    temp_buffer[temp_index] = '-';
                    temp_index--;
                }

                for(int i = temp_index + 1; i < MAX_PRINT_SIZE; i++) {
                    cli_buffer[cli_buffer_index] = temp_buffer[i];
                    cli_buffer_index++;
                }
            }
            // Specifier char - %c
            else if (*string == 'c') {
                string++;
                char x = va_arg(ap, int);

                cli_buffer[cli_buffer_index] = x;
                cli_buffer_index++;
            }

            // Specifier string - %s
            else if (*string == 's') {
                string++;
                char *x = va_arg(ap, char*);

                // Export the data
                while (*x) {
                    cli_buffer[cli_buffer_index++] = *(x++);
                }

                // Make sure string is ended
                cli_buffer[cli_buffer_index] = '\0';
            }
            // Specifier float - %f
            else if (*string == 'f') {
                string++;
                double x = va_arg(ap, double);
                int temp_index = MAX_PRINT_SIZE - 1;
                int flag_negative = 0;
                int dec_count = 9;
                // int cli_buffer_length = 0;

                // // Get float num
                // ftoa(x, temp_buffer, precision);
               
                // Clear printf argument
                reset_arr(argument_string);

                // Convert negative num to positive num
                if(x < 0){
                    x *= (-1);
                    flag_negative = 1;
                    width = width - 1;
                }

                // Set default dec of float num
                if (precision == 0) {
                    precision = 6;
                }

                // Separate int and dec parts
                int int_key = (int)x;
                double dec_key = x - (double)int_key;
                // Move all the dec_key to the left
                int convert_dec_term = (dec_key * 1000000000);

                // get decimal part
                    while (dec_count != 0){
                        if (dec_count <= precision) {
                            temp_buffer[temp_index] = (convert_dec_term % 10) + '0';
                            temp_index--;
                        }

                        dec_count--; // default 6 decimals
                        convert_dec_term /= 10; // get the next dec num
                    }

                    // Print the dot after getting the decimal part
                    if(flag_zero_trig != 1){
                        temp_buffer[temp_index] = '.';
                        temp_index--;
                    }
               
                // get int num part
                    while(int_key != 0){
                        temp_buffer[temp_index] = (int_key % 10) + '0';
                        temp_index--;
                        int_key /= 10; // get the next integer num
                    }

                // Check width
                digit_left = width - (MAX_PRINT_SIZE - 1 - temp_index);

                if (digit_left > 0){
                    for (int i = 0; i < digit_left; i++){
                        if (flag_0 == 1)
                        {
                            temp_buffer[temp_index] = '0';
                            temp_index--;
                        }
                        else
                        {
                            if (flag_negative == 1) {
                                temp_buffer[temp_index] = '-';
                                temp_index--;
                                flag_negative = 0;
                            }
                            temp_buffer[temp_index] = ' ';
                            temp_index--;
                        }
                    }
                }

                // Check negative num
                if (flag_negative == 1) {
                    temp_buffer[temp_index] = '-';
                    temp_index--;
                }

                // Print the argument
                for(int i = temp_index + 1, j = 0; i < MAX_PRINT_SIZE; i++, j++) {
                    cli_buffer[cli_buffer_index] = temp_buffer[i];
                    cli_buffer_index++;
                }
            }
            // Specifier hex - %x
            else if (*string == 'x') {
                string++;
                int x = va_arg(ap, int);
                unsigned int num = x;
                int i = 1;

                // Clear the printf argument
                reset_arr(argument_string);

                while (num != 0) {
                    int digit = num % 16;

                    // get the hex value
                    digit += (digit > 9) ? (-10 + 'a') : '0';
                    temp_buffer[i++] = digit;

                    // Move to next digit
                    num /= 16;
                }

                // read the new converted hex
                for (int j = i - 1; j > 0; j--) {
                    cli_buffer[cli_buffer_index++] = temp_buffer[j];
                }
            }
            else if (*string == '%'){
                cli_buffer[cli_buffer_index] = *(string++);
                cli_buffer_index++;
            }
        }
        else {
            cli_buffer[cli_buffer_index] = *string;
            cli_buffer_index++;
            string++;
        }

        if (cli_buffer_index == MAX_PRINT_SIZE - 1)
            break;
    }

    va_end(ap);

    //Print out formated string
    uart_puts(cli_buffer);

    //Clear buffer after printing
    reset_arr(cli_buffer);
}
