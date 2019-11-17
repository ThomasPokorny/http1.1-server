/********************************************//**
 * HTTP 1.1 Client Header 
 * 
 * @file client.h
 * @author Thomas Robert Pokorny 1527212
 * 
 * @date 4 Nov 2019
 * 
 * @brief defines all the functions for the http client
 * 
 * 
 ***********************************************/
#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdbool.h>

/**
 * @brief holds all connection infomations
 * @var host the host name
 * @var path the doc path to the requested file
 * @var port the connetion port 
 */ 
typedef struct Connections {
   char host[50];
   char path[50];
   char *port;
} Connection;

/**
 * @brief specifies how the server response is saved
 * @var fileName the name of the outputed file
 * @var filePath the path where the doc is saved
 * @var writeToFile if the output is saved to file
 * @var hasFile saves if the users has given the progamm a file name to save the outpur to ("-o ..")
 * @var hasDIr if a directory has been specified by the user ("-d ...")
 */ 
typedef struct SaveInformations {
   char *fileName;
   char *filePath;
   bool writeToFile;
   bool hasFile;
   bool hasDir;
} SaveInformation;

#endif 
