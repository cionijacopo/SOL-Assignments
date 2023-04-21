/*
    Created by Jacopo Ciono on 20/4/2023
    Esercizio 1 - Assignment 9
    Implementazione del server. 
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../includes/Connection.h"
#include "../includes/Utils.h"

#define CHECKNULL(r,c,e) CHECK_EQ_EXIT(e, (r=c), NULL,e,"") 

typedef struct msg {
    int len;
    char *str;
} msg_t;

int main (int argc, chat *argv[]) {
    struct sockadrr_un serv_addr;
    // file descriptor del socket.
    int sockfd;
    SYSCALL_EXIT("socket", sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME) + 1);

    int notused;
    SYSCALL_EXIT("connect", notused, connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)), "connect", "");

    msg_t msg;
    CHECKNULL(str.str, malloc(BUFSIZE), "malloc");

    do {
        char buffer[BUFSIZE];
        memset(str.str, '\0', BUFSIZE);
        // Legge dallo stdin e memorizza in str.str
        if (fgets(str.str, BUFSIZE-1, stdin) == NULL) {
            break;
        }
        if (strncmp(str.str, "quit", 4) == 0) {
            break;
        }
        str.len = strlen(str.str);

        // Comunico al server prima la lunghezza della stringa.
        SYSCALL_EXIT("writen1", notused, writen(sockfd, &str.len, sizeof(int)), "writen1", "");
        // Poi comunico la stringa.
        SYSCALL_EXIT("writen2", notused, writen(sockfd, str.str, str.len), "writen2", "");
        
        int n;
        SYSCALL_EXIT("readn 1", notused, readn(sockfd, &n, sizeof(int)), "read", "");
	    SYSCALL_EXIT("readn 2", notused, readn(sockfd, buffer, n), "read", "");  

        buffer[n] = '\0';
        printf("Risultato: $s\n", buffer);

    } while(1);

    close(sockfd);
    return 0;
}