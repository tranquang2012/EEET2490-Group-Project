#include "string.h"

/**
 * Compute the length of a string
 * @param s: the string whose length is to be computed
 * @return: the length of the string
 */
size_t strlen(const char *s) {
    const char *p = s;
    while (*p != '\0') {
        p++;
    }
    return p - s;
}

/**
 * Locate the first occurrence of a substring in a string
 * @param haystack: the string to be searched
 * @param needle: the substring to search for
 * @return: pointer to the beginning of the found substring or NULL if not found
 */
char *strstr(const char *haystack, const char *needle) {
    if (*needle == '\0') {
        return (char *)haystack;
    }
    
    for (; *haystack != '\0'; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        
        while (*h != '\0' && *n != '\0' && *h == *n) {
            h++;
            n++;
        }
        
        if (*n == '\0') {
            return (char *)haystack;
        }
    }
    
    return NULL;
}

/**
 * Compare the first n characters of two strings
 * @param s1: the first string
 * @param s2: the second string
 * @param n: the number of characters to compare
 * @return: 0 if the strings are equal, a negative value if s1 < s2, or a positive value if s1 > s2
 */
int strncmp(const char *s1, const char *s2, size_t n) {
    while (n > 0 && *s1 != '\0' && *s2 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) {
        return 0;
    }
    
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/**
 * Copy a string from src to dest
 * @param dest: the destination buffer
 * @param src: the source string
 * @return: pointer to the destination buffer
 */
char *strcpy(char *dest, const char *src) {
    char *original_dest = dest;
    while ((*dest++ = *src++)) {
        // Copy each character including the null terminator
    }
    return original_dest;
}
