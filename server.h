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

#endif 
