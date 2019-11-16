/********************************************//**
 *  SOME C UTIL FUNCTIO S 
 * 
 * @file server.h
 * @author Thomas Robert Pokorny 1527212
 * 
 * @date 4 Nov 2019
 * 
 * @brief server functions
 * 
 * 
 ***********************************************/
#ifndef SERVER_H_
#define SERVER_H_

typedef struct ServerConfs {
   int port;
   char *indexFile; 
   char *documentRoot;
} ServerConf;

void printSynopsis();
void debugLog(char *m, char* obj);
int startSocket(ServerConf serverConf);
void closeConnection(FILE *sockfile, FILE *write_sockfile, int client_socket_fd);
void sendInvalidHeader(FILE *write_sockfile, char *code, char *msg);
void sendValidHeader(FILE *write_sockfile, long fileSize);
void sendContent(FILE *sockfile, FILE *write_sockfile);
void receiveSignal(int i);

#endif 
