/********************************************//**
 *  UTIL FUNCTIONS FOR HTTP 1.1 CLient and Server
 * 
 * @file util.c
 * @author Thomas Robert Pokorny 1527212
 * 
 * @date 4 Nov 2019
 * 
 * @brief implements useful string util functions 
 * 
 * 
 ***********************************************/
#include "util.h"

/**
 * @brief checks if a string starts with another string
 * @param pre starts witth
 * @param str the string to check
 * @return true or false if the string starts with pre
 */ 
bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

/**
 * @brief removes a str from a given string
 * @param str the string that we want to remove from
 * @param sub the sub string that we want to remove
 */ 
char *removeString(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}
