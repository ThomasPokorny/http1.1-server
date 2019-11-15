/********************************************//**
 *  HTTP 1.1 SERVER 
 * 
 * @file server.c
 * @author Thomas Robert Pokorny 1527212
 * 
 * @date 4 Nov 2019
 * 
 * @brief the server .c
 * 
 * Calling synopsis:
 * 
 * server [-p PORT] [-i INDEX] DOC_ROOT
 * 
 ***********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include "server.h"
#include "util.h"
#define SA struct sockaddr 

#define ERROR_EXIT(...) { fprintf(stderr, "ERROR: " __VA_ARGS__); exit(EXIT_FAILURE); }

/* SET THIS FLAG TO true IF DEBUG MSGs TO sdtout ARE WANTED */ 
const bool DEBUG = true;

int main(int argc, char *argv[]){

    // NOTE: port index and doc-root
    ServerConf serverConf;
    serverConf.port             = 8080;
    serverConf.indexFile        = "index.html";
    serverConf.documentRoot     = ".";

    // TODO: free memory
    int optIndex = 0;
    int opt;
    while((opt = getopt(argc, argv, "p:i:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'p':
                serverConf.port = atoi(strdup(optarg));
                optIndex = optIndex + 2;
                break;
            case ':':  
                break; 
            case 'i':  
                serverConf.indexFile = strdup(optarg);
                optIndex = optIndex + 2;
                break; 
            case '?':  
                //wrongArgument = true;
                break;   
            default:
                printf("as");
                break;
        }  
    }  

    if (argv[optIndex + 1] == NULL) {
        ERROR_EXIT("Mandatory argument  missing: DOC_ROOT %s\n", strerror(errno));
        exit(1);
    }
    serverConf.documentRoot = argv[(optIndex + 1)];

    if(DEBUG == true) {
        debugLog("index file",serverConf.indexFile);
        debugLog("document root",serverConf.documentRoot);
    }

    /********************************************//**
    *  running the Server 
    * 
    ***********************************************/

    int exitCode = startSocket(serverConf);


    return exitCode;
}

int startSocket(ServerConf serverConf){

    int sockfd; 
    struct addrinfo hints, cli;
    socklen_t len;
    memset(&hints, 0, sizeof(hints));
    // before we work on any logik, first lets build a simple server socket
    hints.ai_family = AF_INET; // create Internet Protocol (IP) socket
    hints.ai_socktype = SOCK_STREAM; // use TCP as transport protocol

    struct sockaddr_in servaddr;

    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(serverConf.port); 

    // SOCKET
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd < 0)
        ERROR_EXIT("socket: %s\n", strerror(errno));
        //BIND
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        ERROR_EXIT("socket bind failed...%s\n", strerror(errno));
    } 
    else
        debugLog("bind", "Socket successfully binded..");


    // LISTEN 
    if ((listen(sockfd, 5)) != 0) { 
        ERROR_EXIT("socket listen failed...%s\n", strerror(errno));
        exit(0); 
    } 
    else
        debugLog("listen", "Socket listening..");
    len = sizeof(cli); 

    // infinite loop that accepts requests from client
    for(;;){
        int client_socket_fd;
        client_socket_fd = accept(sockfd, (SA*)&cli, &len);

        if(DEBUG == true) {
            debugLog("accept", "server waiting for clients to connect");
        }

        FILE *sockfile = fdopen(client_socket_fd, "r");
        FILE *write_sockfile = fdopen(dup(client_socket_fd), "w");

        char buf[1024];
        // NOTE: here we are checking for the request type (which must be GET), the protocol "HTTP/1.1" and the requested file!
        if (fgets(buf, sizeof(buf), sockfile) != NULL){
            if(DEBUG == true) {
                debugLog("client request", buf);
            }

            // NOTE: if the request is not of the type "GET", the server sends a response header with status code 501 (Not implemented) and closes the connection.
            char *reqFile; 
            char *method;
            char * protocol; 

            char * token = strtok(buf, " ");
            int wordCount = 0;
            while( token != NULL ) {
                wordCount ++;
                if(wordCount == 1)
                    method = token;
                else if(wordCount == 2)
                    reqFile = token;
                else if(wordCount == 3)
                    protocol = token;
        
                token = strtok(NULL, " ");
            }

            // NOTE: checking for invalid header
            if(wordCount != 3 || startsWith("HTTP/1.1", protocol) == false){
                if(DEBUG == true) {
                    debugLog("processing request", "400 Bad request");
                }
                sendInvalidHeader(write_sockfile, "400");
            }
            else if(startsWith("GET", method) == false){
                // TODO send 501 and close all connections
                if(DEBUG == true) {
                    debugLog("processing request", "501 not implemented");
                }
                sendInvalidHeader(write_sockfile, "501");               
            }
            // the request is valid!
            else{
                if(DEBUG == true) {
                    debugLog("processing request", "valid Get request!");
                }

                // TODO check file
                sendValidHeader(write_sockfile, "200");

                char *filePath = malloc(strlen(serverConf.documentRoot) + strlen(serverConf.indexFile));
                strcat(filePath, serverConf.documentRoot);
                strcat(filePath, serverConf.indexFile);

                if(DEBUG == true) {
                    debugLog("sending file", filePath);
                }

                FILE *f = fopen(filePath, "r");
                sendContent(write_sockfile, f);

                fclose(f);
                // fprintf(write_sockfile, "%s", "seas");

                fflush(write_sockfile);
                free(filePath);
            }
            closeConnection(sockfile, write_sockfile, client_socket_fd); 
        }

        if(DEBUG == true) {
            debugLog("done request", "server is done processing this request");
        }

        
    }
    
    close(sockfd); 

    return 0;
}

void closeConnection(FILE *sockfile, FILE *write_sockfile, int client_socket_fd){
    fclose(sockfile); 
    fclose(write_sockfile); 
    close(client_socket_fd); 
}

void sendInvalidHeader(FILE *write_sockfile, char *code){
    char *headerLine = malloc(strlen(code) + strlen("HTTP/1.1") + strlen("OK") + 2);
    strcat(headerLine, "HTTP/1.1 ");
    strcat(headerLine, code);
    strcat(headerLine, " ");
    strcat(headerLine, "OK");

    fprintf(write_sockfile, "%s", headerLine);
    fprintf(write_sockfile, "%s", "Connection: close");
    fprintf(write_sockfile, "%s", "\n\r");
    fflush(write_sockfile);

    free(headerLine);
}

void sendContent(FILE *write_sockfile, FILE *f){
    size_t maxlenght = 256;
    char *line = malloc(maxlenght * sizeof(char));

    while (fgets(line, maxlenght, f)) {
        fprintf(write_sockfile, "%s", line);
    }

    free(line);
}

void sendValidHeader(FILE *write_sockfile, char *code){
    char *headerLine = malloc(strlen(code) + strlen("HTTP/1.1") + strlen("OK") + 2);
    strcat(headerLine, "HTTP/1.1 ");
    strcat(headerLine, code);
    strcat(headerLine, " ");
    strcat(headerLine, "OK");

    fprintf(write_sockfile, "%s", headerLine);
    fprintf(write_sockfile, "%s", "\n\r");
    fprintf(write_sockfile, "%s", "Connection: close");
    fprintf(write_sockfile, "%s", "\n\r");
    fprintf(write_sockfile, "%s", "\n\r");

    fflush(write_sockfile);

    free(headerLine);
}

/**
 * 
 * @brief prints the calling synopsis of the http server 
 * @return Void
 **/
void printSynopsis(){
    char *synopsis = "Illegal Arguments given! SYNOPSIS: \nserver [-p PORT] [-i INDEX] DOC_ROOT";
    printf("%s\n", synopsis);
}

void debugLog(char *m, char* obj){
    printf("DEBUG: '%s': ", m);
    printf("%s\n", obj);
}
