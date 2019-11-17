/********************************************//**
 *  SOME C UTIL FUNCTIO S 
 * 
 * @file util.h
 * @author Thomas Robert Pokorny 1527212
 * 
 * @date 4 Nov 2019
 * 
 * @brief util function
 * 
 * 
 ***********************************************/
#ifndef UTIL_H_   
#define UTIL_H_

/* nessesary includes to make the functions work */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief removes a str from a given string
 * @param str the string that we want to remove from
 * @param sub the sub string that we want to remove
 */ 
char *removeString(char *str, const char *sub);

/**
 * @brief checks if a string starts with another string
 * @param pre starts witth
 * @param str the string to check
 * @return true or false if the string starts with pre
 */ 
bool startsWith(const char *pre, const char *str);

#endif 
