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
#include <signal.h>
#include "server.h"
#include "util.h"
#define SA struct sockaddr 

#define ERROR_EXIT(...) { fprintf(stderr, "ERROR: " __VA_ARGS__); exit(EXIT_FAILURE); }

/* SET THIS FLAG TO true IF DEBUG MSGs TO sdtout ARE WANTED */ 
const bool DEBUG = true;

/* this is the runnig flag, if the flag is 0, the server finishes all ungoing requests and terminates with exit code 0 */
static volatile int SERVER_RUNNING = 1;
static volatile int REQUEST_RUNNING = 0;

static int sockfd, client_socket_fd;

int main(int argc, char *argv[]){

    // setting up signal receiving
    signal(SIGINT, receiveSignal);
    signal(SIGTERM, receiveSignal);

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

    // int sockfd; 
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
    // release socket after close 
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
        ERROR_EXIT("socket bind failed...%s\n", strerror(errno));
    }
    //BIND
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        ERROR_EXIT("socket SO_REUSEADDR failed...%s\n", strerror(errno));
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
    while(SERVER_RUNNING == 1){

        // int client_socket_fd;
        client_socket_fd = accept(sockfd, (SA*)&cli, &len);

        if(DEBUG == true) {
            debugLog("accept", "client connection!");
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
            free(token);

            // checking for invalid header
            if(wordCount != 3 || startsWith("HTTP/1.1", protocol) == false){
                if(DEBUG == true) {
                    debugLog("processing request", "400 Bad request");
                }
                sendInvalidHeader(write_sockfile, "400", "Bad Request");
            }
            else if(startsWith("GET", method) == false){
                // TODO send 501 and close all connections
                if(DEBUG == true) {
                    debugLog("processing request", "501 not implemented");
                }
                sendInvalidHeader(write_sockfile, "501", "Not implemented");               
            }
            // processing valid request
            else{

                if(DEBUG == true) {
                    debugLog("processing request", "valid Get request!");
                }

                char *filePath;
                if(reqFile[strlen(reqFile) -1 ] == '/'){
                    filePath = calloc(strlen(serverConf.documentRoot) + strlen(reqFile) + strlen(serverConf.indexFile), sizeof(char)); 
                    strcat(filePath, serverConf.documentRoot);
                    strcat(filePath, reqFile);
                    strcat(filePath, serverConf.indexFile);
                    

                }else{
                    filePath = calloc(strlen(serverConf.documentRoot) + strlen(reqFile), sizeof(char)); 

                    strcat(filePath, serverConf.documentRoot);
                    strcat(filePath, reqFile);

                }
             

                if(DEBUG == true) {
                    debugLog("transmitting file", filePath);
                }

                FILE *f = fopen(filePath, "r");
                free(filePath);

                // if the file does not exist, sending error 404
                if (f == NULL){
                    if(DEBUG == true) {
                        debugLog("cannot find requested file!", "ERROR 404");
                    }
                    sendInvalidHeader(write_sockfile, "404", "Not Found");
                }
                else
                {
                    fseek(f, 0 , SEEK_END);
                    long fileSize = ftell(f);
                    fseek(f, 0 , SEEK_SET);

                    // sending header
                    sendValidHeader(write_sockfile, fileSize);
                    sendContent(write_sockfile, f);
                }
                

                fclose(f);
            }
            fflush(write_sockfile);
            closeConnection(sockfile, write_sockfile, client_socket_fd); 
            REQUEST_RUNNING = 0;
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

void sendInvalidHeader(FILE *write_sockfile, char *code, char *msg){
    char *headerLine = calloc(strlen(code) + strlen("HTTP/1.1") + strlen(msg) + 2, sizeof(char));
    strcat(headerLine, "HTTP/1.1 ");
    strcat(headerLine, code);
    strcat(headerLine, " ");
    strcat(headerLine, msg);

    fprintf(write_sockfile, "%s\n", headerLine);
    fprintf(write_sockfile, "%s\n", "Connection: close");
    fprintf(write_sockfile, "%s", "\n");
    
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

void sendValidHeader(FILE *write_sockfile, long fileSize){
    char *headerLine = "HTTP/1.1 200 OK";

    // geting the UTC timestamp!, in RFC 822 format
    time_t clk = time(NULL);

    struct tm * ptm;
    ptm = gmtime ( &clk );
    time_t utctime = mktime(ptm);

    fprintf(write_sockfile, "%s\n", headerLine);
    fprintf(write_sockfile, "%s", "Date: ");
    fprintf(write_sockfile, "%s", ctime(&utctime));
    fprintf(write_sockfile, "%s", "Content-Length: ");
    fprintf(write_sockfile, "%ld\n", fileSize);
    fprintf(write_sockfile, "%s\n", "Connection: close");
    fprintf(write_sockfile, "%s", "\n");

}

void receiveSignal(int i){
    SERVER_RUNNING = 0;

    if(DEBUG == true) {
        debugLog("signal received: SIGINT or SIGTERM","terminating server!");
    }
    if(REQUEST_RUNNING == 0){
        close(sockfd); 
        close(client_socket_fd);
        exit(0);
    }
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
