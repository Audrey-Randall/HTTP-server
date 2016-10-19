#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include "queue.h"

/* Notes: TCP sockets differ from UDP in that they need a call to listen() and they use recv(), not recvfrom().
    Why are sockaddr_in structs created like that then cast to sockaddr structs?
*/

typedef struct Header{
    char* version;
    char* resp_code;
    char* resp_human;
    char* header1;
    char* val1;
    char* header2;
    char* val2;
    char* header3;
    char* val3;
}Header;

typedef struct Entry{
    char* data;
    int len;
}Entry;

void getType(char* ext, int extLen, Entry* type) {
    FILE* conf = fopen("ws.conf", "r");
    if(conf == NULL) {
        perror("No configuration file found");
        printf("Fatal error: exiting.\n");
        exit(1);
    }
}

void pack_header(Header* header) {
    char* c_type = "Content-Type";
    char* c_len = "Content-Length";
    char* conn = "Connection";
    header->header1 = malloc(strlen(c_type));
    header->header2 = malloc(strlen(c_len));
    header->header3 = malloc(strlen(conn));
    header->header1 = c_type;
    header->header2 = c_len;
    header->header3 = conn;
    Entry* type;
    getType(ext, extLen, type);
    header->val1 = type.data;
    header->val2 = "2";
    header->val3 = "Close";
    header->resp_code = "200";
    header->resp_human = "OK";
    header->version = "HTTP/1.0";
}

int parse_request(char* client_req, char* method, char* uri, char* version) {
    sscanf(client_req, "%s %s %s", method, uri, version);
    if(strcmp(method, "GET")) {
        printf("Unimplemented HTTP method\n");
        return -501;
    } else {
        printf("In parse_request, command was \"%s,%s,%s\"\n", method, uri, version);
    }
    return 0;
}

int sendFile(int client_fd, char* filepath, Header* header) {
    FILE* response = fopen(filepath, "rb");
    char* buffer;
    struct sockaddr_in remoteSock;
    int fileSize = 0;
    int n;
    int i;
    int is404;
    char* header_str;

    if(response == NULL) {
        perror("Open file error in sendFile");
        is404 = 1;
        header->resp_code = malloc(3);
        header->resp_code = "404";
        char* msg = "File not found";
        header->resp_human = malloc(strlen(msg));
        header->resp_human = msg;
    }

    if(is404) {
        response = fopen("404.html", "rb");
        if(response == NULL) {
            printf("AAAAARGH THE ERROR MESSAGE HAS AN ERROR\n");
            return -1;
        }
        printf("response is %p\n", response);
    }


    fseek(response, 0, SEEK_END);
    fileSize = ftell(response);
    rewind(response);
    header->val2 = malloc(log10(fileSize));
    sprintf(header->val2, "%d", fileSize);

    buffer = malloc(fileSize);
    bzero(buffer, fileSize);

    header_str = malloc(1024);
    bzero(header_str, 1024);
    sprintf(header_str, "%s %s %s\r\n%s: %s\r\n%s: %s\r\n%s: %s\r\n\r\n",
            header->version, header->resp_code, header->resp_human, header->header1, header->val1,
            header->header2, header->val2, header->header3, header->val3);

    //Not sure if this still needs to be in a while loop
    while ((n = fread(buffer, sizeof(char), fileSize, response)) > 0) {
        char* payload = malloc(sizeof(buffer)+sizeof(header_str));
        strncpy(payload, header_str, strlen(header_str));
        strcat(payload, buffer);
        printf("File opened, header_str is %s and file contents are %s\n", header_str, buffer);
        send(client_fd, payload, strlen(payload), 0);
        bzero(buffer, sizeof(buffer));
    }
    if (n < 0) printf("Read error\n");
    return 0;
}

int main(int argc, char* argv[]) {
    struct sockaddr_in server, client;
    int sock_fd, client_fd, read_size;
    socklen_t sockaddr_len;
    char* client_req = malloc(1024);
    char* method = malloc(4);
    char* uri = malloc(1086); //2000-2-4-8
    char* version = malloc(8);
    bzero(client_req, 1024);
    bzero(method, 4);
    bzero(uri, 1086);
    bzero(version, 8);

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_port= htons(8080);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddr_len = sizeof(server);

    if(bind(sock_fd,(struct sockaddr *)&server , sizeof(server)) < 0) {
        perror("Bind error");
        return 1;
    }

    if(listen(sock_fd, 3) < 0) {
        //backlog is 3 because the example is 3
        perror("Listen error");
        return 1;
    }

    if((client_fd = accept(sock_fd, (struct sockaddr *)&client, &sockaddr_len)) < 0) {
        perror("Accept error");
        return 1;
    }
    while((read_size = recv(client_fd , client_req , 2000 , 0)) > 0 ) {
        //Echo message back. Write replaces LF with CRLF if not already there
        //send(client_fd, client_req, strlen(client_req), 0);
        //parse request. TODO: handle security issues here
        if(parse_request(client_req, method, uri, version) < 0) {
            printf("Parse error\n");
            //send error consistent with return value
        } else {
            //TODO: create thread to run sendFile here
            Header header;
            pack_header(&header);
            sendFile(client_fd, uri, &header);
        }
        bzero(client_req, 2000);
        fflush(stdout);
    }

    if(read_size == 0) {
        printf("Client disconnected\n");
        //TODO: figure out why free errors exist here
        free(method);
        free(uri);
        free(version);
        fflush(stdout);
    } else if(read_size < 0) {
        perror("Recv failed");
        return 1;
    }
    return 0;
}
