/*
    Created by Jacopo Cioni on 21/4/2023
    Esercizio 1_v2 - Assignment 9
    In questo caso il server non risponde al client.
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "../includes/Connection.h"
#include "../includes/Utils.h"

int main (int argc, char *argv[]) {
    struct sockaddr_un serv_addr;
    int sockfd;
    SYSCALL_EXIT("socket", sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int notused, n;
    SYSCALL_EXIT("connect", notused, connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "connect", "");

    char buf[BUFSIZE];

    do {
        memset(buf, '\0', BUFSIZE);
        if (fgets(buf, BUFSIZE, stdin) == NULL) {
            break;
        }
        if (strncmp(buf, "quit", 4) == 0) {
            break;
        }

        SYSCALL_EXIT("writen", notused, writen(sockfd, buf, strlen(buf)), "writen", "");
        SYSCALL_EXIT("read", n, read(sockfd, buf, BUFSIZE), "readn", "");
        buf[n] = '\0';
        printf("Risultato: %s\n", buf);
    } while(1);
    close(sockfd);
    return 0;
}