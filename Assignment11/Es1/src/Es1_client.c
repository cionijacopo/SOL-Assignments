/*
    Created by Jacopo Cioni on 28/4/2023
    Esercizio 1 - Assignment 1
    Convertire la stringa da minuscola a maiuscola con un server multi-threaded.
    Questo è il client per l'esercizio 11, copiato dall'esercizio 9.
*/

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../includes/connection.h"
#include "../includes/utils.h"

int main(int argc, char *argv[]) {
    if(argc == 1) {
        fprintf(stderr, "Utilizzare: %s <stringa> ...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un serv_addr;
    int sockfd;
    SYSCALL_EXIT("socket", sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");

    // Setto l'indirizzo
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;    
    strncpy(serv_addr.sun_path,SOCKNAME, strlen(SOCKNAME)+1);

    int notused;
    SYSCALL_EXIT("connect", notused, connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "connect", "");

    char *buffer = NULL;
    for(int i=1; i<argc; i++) {
        int n = strlen(argv[i])+1;

        // Passo prima la dimensione al server e poi la stringa
        SYSCALL_EXIT("writen", notused, writen(sockfd, &n, sizeof(int)), "write", "");
	    SYSCALL_EXIT("writen", notused, writen(sockfd, argv[i], n*sizeof(char)), "write", "");

        buffer = realloc (buffer, n*sizeof(char));
        if(!buffer) {
            perror("realloc");
            fprintf(stderr, "Memoria esaurita..\n");
            break;  
        }

        SYSCALL_EXIT("readn", notused, readn(sockfd, &n, sizeof(int)), "read","");
	    SYSCALL_EXIT("readn", notused, readn(sockfd, buffer, n*sizeof(char)), "read","");
        buffer[n] = '\0';
        printf("Risultato: %s\n", buffer);
    }
    close(sockfd);
    if (buffer) {
        free(buffer);
    }
    return 0;
}