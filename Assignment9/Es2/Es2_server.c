/*
    Created by Jacopo Cioni on 20/4/2023
    Esercizio 2 - Assignment 9
    Convertire la stringa da minuscola a maiuscola con un server multi-threaded.
*/

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

#include "../includes/Connection.h"
#include "../includes/Utils.h"

// Abbiamo un thread per connessione.
// I thread sono lanciati in modalità detached.

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

void *strUP(void *arg) {
    // Asserzione. Do per necessario l'argomento.
    assert(arg);
    long connfd = (long)arg;

    do {
        msg_t str;
        int n;
        SYSCALL_EXIT("readn", n, readn(connfd, &str.len, sizeof(int)), "readn", "");
        if (n == 0) {
            // Non ho letto niente.
            break;
        }
        str.str = calloc((str.len), sizeof(char));
        if(!str.str) {
            perror("calloc");
            fprintf(stderr, "Memoria esaurita..\n");
            break;
        }
        SYSCALL_EXIT("readn", n, readn(connfd, str.str, str.len*sizeof(char)), "readn", "");
        toup(str.str);

        SYSCALL_EXIT("writen", n, writen(connfd, &str.len, sizeof(int)), "writen", "");
        SYSCALL_EXIT("writen", n, writen(connfd, str.str, str.len*sizeof(char)), "writen", "");
        free(str.str);
    } while(1);
    close(connfd);
    return NULL;
}

void spawn_thread(long connfd) {
    // Definisco gli attributi del thread
    pthread_attr_t thread_attr;
    // Definisco l'id del thread
    pthread_t thread_id;

    if(pthread_attr_init(&thread_attr) != 0) {
        fprintf(stderr, "pthread_attr_init FALLITA.\n");
        close(connfd);
        return;
    }
    // Settiamo il thread in modalità detached. Il trhead detached una
    // volta terminato restituisce le risorse automaticamente al sistema
    // senza bisogno che un altro thread faccia la join per aspettarlo.
    if(pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        fprintf (stderr, "pthread_attr_setdetatchedstate FALLITA\n");
        pthread_attr_destroy(&thread_attr);
        close(connfd);
        return;
    }
    // Passo il descrittore del socket come argomento alla funzione.
    if (pthread_create(&thread_id, &thread_attr, strUP, (void*)connfd) != 0) {
        fprintf(stderr, "pthread_create FALLITA");
        pthread_attr_destroy(&thread_attr);
        close(connfd);
        return;
    }
}

int main (int argc, char *argv[]) {
    //Faccio la cleanup e registro l'errore in caso di problemi
    cleanup();
    atexit(cleanup);

    int listenfd;
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");

    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, strnlen(SOCKNAME)+1);

    int notused;
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)), "bind", "");
    // MAXBACKLOG vale 32, in questo modo gli dico che può ascoltare 32 processi contemporaneamente
    SYSCALL_EXIT("listen", notused, listen(listenfd, MAXBACKLOG), "listen", "");

    while(1) {
        long connfd;
        SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*)NULL, NULL), "accept", "");
        // Connessione accettata.
        spawn_thread(connfd);
    }
    return 0;
}