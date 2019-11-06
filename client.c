/********************************************//**
 *  HTTP 1.1 CLIENT 
 * 
 * @file client.c
 * @author Thomas Robert Pokorny 1527212
 * 
 * @date 4 Nov 2019
 * 
 * @brief the client .c
 * 
 * Calling synopsis:
 * 
 * client [-p PORT] [ -o FILE | -d DIR ] URL
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
#define _GNU_SOURCE

#define ERROR_EXIT(...) { fprintf(stderr, "ERROR: " __VA_ARGS__); exit(EXIT_FAILURE); }

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

const bool DEBUG = true;

void printSynopsis();
void debugLog(char *m, char* obj);
void getHostandPath(char * connectionUrl, char *hostname, char *path);
int connectToHost(SaveInformation saveInformation, Connection connection);
void createOutputSettings(SaveInformation *saveInformation, Connection connection);
bool startsWith(const char *pre, const char *str);
char *removeString(char *str, const char *sub);


int main(int argc, char *argv[]){

    int exitCode = EXIT_SUCCESS; 
    
    char *port = "80"; // the default port

    // structs needed
    Connection connection;
    SaveInformation saveInformation;
    saveInformation.writeToFile = false;
    saveInformation.hasFile     = false;
    saveInformation.hasDir      = false;
    //  [ -o FILE | -d DIR ]

    int opt;
    bool wrongArgument = false; 
    while((opt = getopt(argc, argv, "o:d:p:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'p':
                port = strdup(optarg);
                break;
            case ':':  
                printf("option needs a value: %c\n", optopt);
                break; 
            case 'o':  
                saveInformation.fileName    = strdup(optarg);
                saveInformation.writeToFile = true;
                saveInformation.hasFile     = true;
                break; 
            case 'd':  
                saveInformation.filePath    = strdup(optarg);
                saveInformation.writeToFile = true;
                saveInformation.hasDir      = true;
                break; 
            case '?':  
                //wrongArgument = true;
                break;   
            default:
                printf("as");
                break;
        }  
    }  

    if(wrongArgument == true){
        printSynopsis();
        ERROR_EXIT("parsing arguments: %s\n", "wrong arguments");    
    }    

    char *connectionUrl = argv[(argc -1)];
    if(DEBUG == true)    
        debugLog("the given connection url",connectionUrl);
  
    getHostandPath(connectionUrl, connection.host, connection.path);
    connection.port = port;

    //
    if(saveInformation.writeToFile == true){
        createOutputSettings(&saveInformation, connection);
    }

    if(DEBUG == true) {
        debugLog("host",connection.host);
        debugLog("path",connection.path);
        debugLog("port",connection.port);
    }

    /********************************************//**
    *  CONNECTION 
    * 
    ***********************************************/

    connectToHost(saveInformation, connection);


    return exitCode;
}

int connectToHost(SaveInformation saveInformation, Connection connection){
       
    struct addrinfo hints, *ai;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // create Internet Protocol (IP) socket
    hints.ai_socktype = SOCK_STREAM; // use TCP as transport protocol

    int res = getaddrinfo(connection.host, connection.port, &hints, &ai);
    if (res != 0)
        ERROR_EXIT("getaddrinfo: %s\n", gai_strerror(res));

    int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sockfd < 0)
        ERROR_EXIT("socket: %s\n", strerror(errno));

    if (connect(sockfd, ai->ai_addr, ai->ai_addrlen) < 0)
        ERROR_EXIT("connect: %s\n", strerror(errno));

    FILE *sockfile = fdopen(sockfd, "r+");
    if (sockfile == NULL)
        ERROR_EXIT("fdopen: %s\n", strerror(errno));

    // send a GET request to the server:
    char *headerPart1 = "GET ";
    char *headerPart2 = " HTTP/1.1\r\nHost: ";
    char *closeConnection = "\r\nConnection: close";
    char *headerPart3 = "\r\n\r\n";

    char *request = malloc(strlen(connection.path) + strlen(connection.host) + strlen(headerPart1) + strlen(headerPart2) + strlen(headerPart3) + strlen(closeConnection)); 
    strcat(request, headerPart1); 
    strcat(request, connection.path); 
    strcat(request, headerPart2);
    strcat(request, connection.host); 
    strcat(request, closeConnection); 
    strcat(request, headerPart3); 

    if (fputs(request, sockfile) == EOF)
        ERROR_EXIT("fputs: %s\n", strerror(errno));
    if (fflush(sockfile) == EOF)
        ERROR_EXIT("fflush: %s\n", strerror(errno));

    free(request);
    char buf[1024];

    bool isContent = false;
    if(saveInformation.writeToFile == true){
        
        if(saveInformation.hasDir == true){

            DIR* dir = opendir(saveInformation.filePath);
            bool createDir = true;
            if (dir) {
                closedir(dir);
                createDir = true;
            }

            if(createDir == true){
                mkdir(saveInformation.filePath, 0700);
            }

            if(DEBUG == true) {
                debugLog("saving in directory", saveInformation.filePath);
            }
        }

        if(DEBUG == true) {
            debugLog("saving response into file", saveInformation.fileName);
        }
        FILE * outFile = fopen(saveInformation.fileName, "w+");

        bool headerChecked = false;
        while (fgets(buf, sizeof(buf), sockfile) != NULL){
            if(headerChecked == false){
                bool isHeaderCorrect = startsWith("HTTP/1.1", buf);

                if(DEBUG == true) {
                    debugLog("header response, first line", buf);
                }

                // NOTE: if the header is not correct the client terminated with exit code 2
                if(isHeaderCorrect == false){
                    printf("Protocol error!\n");
                    return 2;
                }

                // checking the response status
                char *statusStr = removeString(buf, "HTTP/1.1");

                if(DEBUG == true) {
                    debugLog("status code", statusStr);
                }

                long status;
                char *ptr;
                status = strtol(statusStr, &ptr, 10);

                if(status != 200){
                    printf("ERROR: invalid status received from server:%s\n(declining all other status codes than '200'\n", statusStr);
                    return 3;
                }

                headerChecked = true;
            }

            if(isContent == true)
                fputs(buf, outFile);
            if(buf[0] == '\r')
                isContent = true;
        }
        fclose(outFile);
        free(saveInformation.fileName);
    }
    // print do stdout
    else{
        bool headerChecked = false;
        while (fgets(buf, sizeof(buf), sockfile) != NULL){
            if(headerChecked == false){
                bool isHeaderCorrect = startsWith("HTTP/1.1", buf);

                if(DEBUG == true) {
                    debugLog("header response, first line", buf);
                }

                // NOTE: if the header is not correct the client terminated with exit code 2
                if(isHeaderCorrect == false){
                    printf("Protocol error!\n");
                    return 2;
                }

                // checking the response status
                char *statusStr = removeString(buf, "HTTP/1.1");

                if(DEBUG == true) {
                    debugLog("status code", statusStr);
                }

                long status;
                char *ptr;
                status = strtol(statusStr, &ptr, 10);

                if(status != 200){
                    printf("ERROR: invalid status received from server:%s\n(declining all other status codes than '200'\n", statusStr);
                    return 3;
                }

                headerChecked = true;
            }

            if(isContent == true)
                fputs(buf, stdout);
            if(buf[0] == '\r')
                isContent = true;
        }
    }
    

    fclose(sockfile); 

    return 0;
}

/**
 * 
 * @brief prints the calling synopsis of the http client 
 * @return Void
 **/
void printSynopsis(){
    char *synopsis = "Illegal Arguments given! SYNOPSIS: \nclient [-p PORT] [ -o FILE | -d DIR ] URL";
    printf("%s\n", synopsis);
}

void debugLog(char *m, char* obj){
    printf("DEBUG: '%s': ", m);
    printf("%s\n", obj);
}

/**
 * 
 * PRECONDITION: a valid string that starts with "http://"
 * @return the hostname for a given http url
 */ 
void getHostandPath(char * connectionUrl, char *hostname, char *path){
    // ; / ? : @ = &
    int runningIndex = 0;
    int pathIndex = 0;

    // determing the host
    for (int i = 7; connectionUrl[i]; i++) {
        if(connectionUrl[i] == ';' || connectionUrl[i] == '/' || connectionUrl[i] == '?' || connectionUrl[i] == ':' || connectionUrl[i] == '=' || connectionUrl[i] == '&'){
            pathIndex = i;
            break;
        }
        hostname[runningIndex] = connectionUrl[i];
        runningIndex++;
    }
    hostname[runningIndex] = '\0';

    if(pathIndex == 0){
        path[0] = '/';
        path[1] = '\0';
    }
    else{
        runningIndex = 0;
        // determing the path 
        for (int i = pathIndex; connectionUrl[i]; i++) {
            path[runningIndex] = connectionUrl[i];
            runningIndex++;
        }
        path[runningIndex] = '\0';
    }
}

/**
 */
void createOutputSettings(SaveInformation *saveInformation, Connection connection){
    
    char *indexFile;
    bool allocated = false;

    // seacrh for the last '/' in the path, this is going to be the file name
    long pos = strrchr(connection.path, '/') -connection.path;

    if(saveInformation->hasFile == true){
        indexFile = malloc(strlen(saveInformation->fileName)); 
        indexFile = saveInformation->fileName;
        allocated = true;
    }else if (pos != 0){

        indexFile = malloc(strlen(connection.path) - pos); 
        int runningIndex = 0;
        for (int i = (pos +1); connection.path[i]; i++) {
            indexFile[runningIndex] = connection.path[i];
            runningIndex ++;
        }
        allocated = true;

        if(DEBUG == true) {
            debugLog("generated file name", indexFile);
        }

    }else{
        indexFile = "index.html";
    }


    char *filePath;
    if( 1 == 2 && saveInformation->hasDir == true && saveInformation->filePath[0] != '.'){
        
        filePath = malloc(strlen(saveInformation->filePath) + strlen(indexFile) + 1); 
        strcat(filePath, saveInformation->filePath); 
        strcat(filePath, "/"); 
        strcat(filePath, indexFile); 
    }else{
        filePath = malloc(strlen(indexFile));  
        strcat(filePath, indexFile); 
    }

    saveInformation->fileName = malloc(strlen(filePath));
    // saveInformation->fileName = filePath;
    strcpy (saveInformation->fileName, filePath);
    free(filePath);
    if(allocated == true)
        free(indexFile);
}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

char *removeString(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

