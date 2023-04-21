/*
    Created by Jacopo Cioni on 21/4/2023
    Esercizio 1_v2 - Assignment 9
    In questo caso il server non risponde al client.
*/

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include "../includes/Connection.h"
#include "../includes/Utils.h"

void cleanup() {
    unlink(SOCKNAME);
}

int cmd(int connfd) {
    int notused;

    int pid = fork();
    if (pid == 0) {
        // redirigo input output ed error sul descrittore associato
        // alla connessione con il client
        SYSCALL_EXIT("dup2", notused, dup2(connfd, 0), "dup2 child (1)", "");
        SYSCALL_EXIT("dup2", notused, dup2(connfd, 1), "dup2 child (2)", "");
        SYSCALL_EXIT("dup2", notused, dup2(connfd, 2), "dup2 child (2)", "");
      
        execl("/usr/bin/bc", "bc", "-lq", NULL);
        return -1;
    } 
    SYSCALL_EXIT("wait", notused, wait(NULL), "wait", "");
    printf("Child terminato\n");

    return 0;
}

int main(int argc, char *argv[]) {
    // Cancello il socket se esiste.
    cleanup();
    // Se qualcosa va storto
    atexit(cleanup);

    int listenfd;
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");

    // Setto l'indirizzo
    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int notused;
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)), "bind", "");
    // Setto il socket in modalità passiva
    SYSCALL_EXIT("listen", notused, listen(listenfd, 1), "listen", "");

    int connfd;
    do {
        SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*)NULL, NULL), "accept", "");
        if (cmd(connfd) < 0) {
            fprintf(stderr, "Errore nell'esecuzione del comando\n");
            break;
        }
        close(connfd);
        printf("Connessione effettuata\n");
    } while(1);
    close(listenfd);
    return 0;    
}

