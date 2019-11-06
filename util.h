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


char *removeString(char *str, const char *sub);
bool startsWith(const char *pre, const char *str);

#endif 
