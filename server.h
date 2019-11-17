/********************************************//**
 *  HEADER FOR the HTTP1.1 Server 
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

/**
 * @brief holds all server infomations
 * @var port the server  port 
 * @var indexFile the index file
 * @var documentRoot the document root for the server
 */ 
typedef struct ServerConfs {
   int port;
   char *indexFile; 
   char *documentRoot;
} ServerConf;

/**
 * @brief the server is started, and awaits clients
 * @param serverConf server configuration: port, index file and docuemtn root
 */ 
int startSocket(ServerConf serverConf);

/**
 * @brief closes open connections to a client of the socket
 * @param sockFile the receiving socket
 * @param write_sockFile the out socket file to the client
 * @param client_socket_fd the client file descriptor
 */ 
void closeConnection(FILE *sockfile, FILE *write_sockfile, int client_socket_fd);

/**
 * @brief sends a invalid header ( 404, 400, 501, ..) to the client
 * @param write_sockfile the out socket file to the client
 * @param code the http return code, 404 , 400, 501, ..
 * @param msg the message: "Not Found", "Not implemented", "Bad Request" , ...
 */ 
void sendInvalidHeader(FILE *write_sockfile, char *code, char *msg);

/**
 * @brief writes a valid header to the client socket
 * @param write_sockfile the out socket file to the client
 * @param fileSize the filesize that is going to be transmitted
 */ 
void sendValidHeader(FILE *write_sockfile, long fileSize);

/**
 * @brief writes a file line by line to the client write socket file
 * @param write_sockfile the out socket file to the client
 * @param f the file that is going to be transmitted
 */ 
void sendContent(FILE *sockfile, FILE *write_sockfile);

/**
 * @brief receives signals SIGINT, SIGTERM. if the server is not currently processing a client, the exit() function is called
 */ 
void receiveSignal(int i);

/**
 * 
 * @brief prints the calling synopsis of the http server 
 * @return Void
 **/
void printSynopsis();

/**
 * @brief logs a debug message to stdout
 * @param m the message titel
 * @param obj the message
 */ 
void debugLog(char *m, char* obj);


#endif 
