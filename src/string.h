#ifndef STRING_H
#define STRING_H

#include <stddef.h>  // For size_t

// Function declarations for string operations

/**
 * Compute the length of a string
 * @param s: the string whose length is to be computed
 * @return: the length of the string
 */
size_t strlen(const char *s);

/**
 * Copy a string from src to dest
 * @param dest: the destination buffer
 * @param src: the source string
 * @return: pointer to the destination buffer
 */
char *strcpy(char *dest, const char *src);

/**
 * Locate the first occurrence of a substring in a string
 * @param haystack: the string to be searched
 * @param needle: the substring to search for
 * @return: pointer to the beginning of the found substring or NULL if not found
 */
char *strstr(const char *haystack, const char *needle);

/**
 * Compare two strings
 * @param s1: first string to compare
 * @param s2: second string to compare
 * @return: integer less than, equal to, or greater than zero if s1 is found,
 * respectively, to be less than, to match, or be greater than s2
 */
int strcmp(const char *s1, const char *s2);

/**
 * Compare two strings up to a given number of characters
 * @param s1: first string to compare
 * @param s2: second string to compare
 * @param n: the maximum number of characters to compare
 * @return: integer less than, equal to, or greater than zero if s1 is found,
 * respectively, to be less than, to match, or be greater than s2
 */
int strncmp(const char *s1, const char *s2, size_t n);

#endif  // STRING_H
