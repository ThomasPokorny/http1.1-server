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

/**
 * 
 * @brief prints the calling synopsis of the http client 
 * @return Void
 **/
void printSynopsis();

/**
 * @brief logs a debug message to stdout
 * @param m the message titel
 * @param obj the message
 */ 
void debugLog(char *m, char* obj);

/**
 * 
 * PRECONDITION: a valid string that starts with "http://"
 * @param connectionUrl from this url the hostname and file path is determined
 * @param hostName gets set from the connection Url 
 * @param path gets set from the connection Url 
 * POSTCONDITION:  hostName and path is set 
 */ 
void getHostandPath(char * connectionUrl, char *hostname, char *path);

/**
 * @brief connects to the server, and either saves the response to a file or prints it to stdout ( this information is given in  saveInformation)
 * @param saveInformation hot the output is saved
 * @param connection the connection information, that includes a port, host and path
 * @return the return code
 */ 
int connectToHost(SaveInformation saveInformation, Connection connection);

/**
 * 
 * @brief generates the file name and directory for the saved file
 * @return Void
 * @param saveInformation this struct specifies how the output is saved
 * @param connection the connection information, that includes a port, host and path
 **/
void createOutputSettings(SaveInformation *saveInformation, Connection connection);

#endif 
