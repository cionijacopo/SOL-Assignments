/*
     Created by Jacopo Cioni on 21/4/2023
     Esercizio 3 - Assignment 9
     Server single threaded che gestisce le richieste mediante select.
*/

#define _POSIX_C_SOURCE  200112L
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/select.h>

#include "../includes/Connection.h"
#include "../includes/Utils.h"

typedef struct msg {
    int len;
    char *str;
} msg_t;

void cleanup() {
    unlink(SOCKNAME);
}

// Convertitore di caratteri minuscoli in maiuscoli
void toup(char *str) {
    char *p = str;
    while(*p != '\0') {
        if (islower(*p)) {
            toupper(*p);
        }
        *p++;
    }
}

int cmd(long connfd) {
    msg_t str;
    if (readn(connfd, &str.len, sizeof(int))<=0) return -1;
    str.str = calloc((str.len), sizeof(char));
    if (!str.str) {
	perror("calloc");
	fprintf(stderr, "Memoria esaurita....\n");
	return -1;
    }		        
    if (readn(connfd, str.str, str.len*sizeof(char))<=0) return -1;
    toup(str.str);
    if (writen(connfd, &str.len, sizeof(int))<=0) { free(str.str); return -1;}
    if (writen(connfd, str.str, str.len*sizeof(char))<=0) { free(str.str); return -1;}
    free(str.str);
    return 0;
}

// ritorno l'indice massimo tra i descrittori attivi
int updatemax(fd_set set, int fdmax) {
    for(int i=(fdmax-1);i>=0;--i)
	if (FD_ISSET(i, &set)) return i;
    assert(1==0);
    return -1;
}

int main(int argc, char *argv[]) {
    // Pulizia e segnatore errore
    cleanup();
    atexit(cleanup);

    int listenfd;
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");
    
    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;    
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int notused;

    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*)&serv_addr,sizeof(serv_addr)), "bind", "");
    SYSCALL_EXIT("listen", notused, listen(listenfd, MAXBACKLOG), "listen", "");

    // Definisco i set per la select.
    fd_set set, tmpset;
    // Azzero sia il master set sia quello temporaneo usato per la select
    FD_ZERO(&set);
    FD_ZERO(&tmpset);

    // Metto a 1 il bit corrispondente al file descriptor listenfd
    // nella maschera set
    FD_SET(listenfd, &set);

    // Tengo traccia del file descriptor con id più grande
    int fdmax = listenfd;

    // For infinito
    for(;;) {
        // Copio il set nella variabile temporanea per il set.
        tmpset = set;
        if (select(fdmax+1, &tmpset, NULL, NULL, NULL) == -1) {
            perror("select");
            return -1;
        }

        // Dobbiamo capire da quale file descriptor abbiamo ricevuto la richiesta
        for (int i = 0; i<=fdmax; i++) {
            if(FD_ISSET(i, &tmpset)) {
                long connfd;
                if (i == listenfd) {
                    // Nuova richiesta di connessione
                    SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*)NULL, NULL), "accept", "");
                    // Aggiungo il descrittore al master set
                    FD_SET(connfd, &set);
                    if (connfd > fdmax) {
                        // ricalcolo il massimo;
                        fdmax = connfd;
                    }
                    continue;
                }
                // è una nuova richiesta da un client già connesso
                connfd = i;
                // eseguo un comando e se c'è un errore lo tolgo dal master set
                if(cmd(connfd) < 0) {
                    close(connfd);
                    FD_CLR(connfd, &set);
                    // Controllo se devo riaggiornare il massimo. 
                    if (connfd == fdmax) {
                        fdmax = updatemax(set, fdmax);
                    }
                }
            }
        }
    }
    close(listenfd);
    return 0;
}