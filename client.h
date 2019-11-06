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
#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdbool.h>

typedef struct Connections {
   char host[50];
   char path[50];
   char *port;
} Connection;

typedef struct SaveInformations {
   char *fileName;
   char *filePath;
   bool writeToFile;
   bool hasFile;
   bool hasDir;
} SaveInformation;

void printSynopsis();
void debugLog(char *m, char* obj);
void getHostandPath(char * connectionUrl, char *hostname, char *path);
int connectToHost(SaveInformation saveInformation, Connection connection);
void createOutputSettings(SaveInformation *saveInformation, Connection connection);

#endif 
