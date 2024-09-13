
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

// Function reverse string
void reverse(char *str, int len);

// Function convert decimal to string
int itoa(int x, char str[], int d);

// User-defined pow() function
// double pow(double x, int n);
double pow(double base, double power);  

// Function to count the input decimal
int count_decimal_places(double num);

// Function to check if input is digit
int isDigit(char ch);

// Fucntion convert hex to sting
void ftoa(double n, char* res, int afterpoint);

// Function checking specifier inside argument to get width and precision
void argument_check_sub_specfier(char* description, int* width, int* precision, int* flag_zero, int* flag_zero_pre);


void printf(char *string,...);

