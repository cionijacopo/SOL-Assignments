/*
    Created by Jacopo Cioni on 24/4/2023
    Esercizio 2 - Assignment 10
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
// Mi serve per i segnali
#include<signal.h>

#include "../includes/Connection.h"
#include "../includes/Utils.h"

// Mi serve per poter fare la gestione dei segnali.
#define _POSIX_C_SOURCE 200112L

// Abbiamo un thread per connessione.
// I thread sono lanciati in modalità detached.

typedef struct msg {
    int len;
    char *str;
} msg_t;

static volatile sig_atomic_t termina = 0;
static void sighandler(int usless) {
    termina = 1;
}

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


// Qui adesso dovroò gestire i segnali che saranno presenti per ogni thread.
int spawn_thread(long connfd) {
    // Definisco gli attributi del thread
    pthread_attr_t thread_attr;
    // Definisco l'id del thread
    pthread_t thread_id;
    // Definisco il set per la maschera dei segnali
    sigset_t mask, oldmask;
    // Setto la maschera
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    // SIGTERM è un segnale utilizzato per la terminazione di un programma.
    // A differenza di SIGKILL può essere gestito, bloccato o ignorato. 
    // È il metodo migliore per chiedere ad un programma di terminare.
    sigaddset(&mask, SIGTERM);
    // SIGHUP è un segnale utilizzato per riportare che il terminale
    // dell'utente è disconnesso (ad esempio se una connessione si è interrotta).
    sigaddset(&mask, SIGHUP);

    if(pthread_sigmask(SIG_BLOCK, &mask, &oldmask) != 0) {
        fprintf("FATAL ERROR, pthread_sigmask\n");
        close(connfd);
        return -1;
    }

    if(pthread_attr_init(&thread_attr) != 0) {
        fprintf(stderr, "pthread_attr_init FALLITA.\n");
        close(connfd);
        return -1;
    }
    // Settiamo il thread in modalità detached. Il trhead detached una
    // volta terminato restituisce le risorse automaticamente al sistema
    // senza bisogno che un altro thread faccia la join per aspettarlo.
    if(pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) != 0) {
        fprintf (stderr, "pthread_attr_setdetatchedstate FALLITA\n");
        pthread_attr_destroy(&thread_attr);
        close(connfd);
        return -1;
    }
    // Passo il descrittore del socket come argomento alla funzione.
    if (pthread_create(&thread_id, &thread_attr, strUP, (void*)connfd) != 0) {
        fprintf(stderr, "pthread_create FALLITA");
        pthread_attr_destroy(&thread_attr);
        close(connfd);
        return -1;
    }
    
    // Ripristino la maschera vecchia(?)
    if (pthread_sigmask(SIG_SETMASK, &oldmask, NULL) != 0) {
        fprintf(stderr, "FATAL ERROR\n");
        close(connfd);
        return -1;
    }
    return 0;
}

int main (int argc, char *argv[]) {
    //Faccio la cleanup e registro l'errore in caso di problemi
    cleanup();
    atexit(cleanup);

    sigset_t mask, oldmask;
    sigemptyset(&mask);   
    sigaddset(&mask, SIGINT); 
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGHUP);

    if(pthread_sigmask(SIG_BLOCK, &mask, &oldmask) != 0) {
        fprintf("FATAL ERROR, pthread_signmask\n");
        return EXIT_FAILURE;
    }

    //Installo il signal handler per tutti i segnali che mi interessano.
    struct sigaction sa;
    // Resetto la struttura
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sighandler;
    int notused;
    SYSCALL_EXIT("sigaction", notused, sigaction(SIGINT, &sa, NULL), "sigaction", "");
    SYSCALL_EXIT("sigaction", notused, sigaction(SIGQUIT, &sa, NULL), "sigaction", "");
    SYSCALL_EXIT("sigaction", notused, sigaction(SIGTERM, &sa, NULL), "sigaction", "");
    SYSCALL_EXIT("sigaction", notused, sigaction(SIGHUP, &sa, NULL), "sigaction", "");

    int listenfd;
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");

    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, strnlen(SOCKNAME)+1);

    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)), "bind", "");
    // MAXBACKLOG vale 32, in questo modo gli dico che può ascoltare 32 processi contemporaneamente
    SYSCALL_EXIT("listen", notused, listen(listenfd, MAXBACKLOG), "listen", "");

    if(pthread_sigmask(SIG_SETMASK, &oldmask, NULL) != 0) {
        fprintf(stderr, "FATAL ERROR\n");
        return EXIT_FAILURE;
    }

    int r = 0;
    while(!termina) {
        long connfd = -1;
        if(!termina && (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1) {
            if(errno = EINTR) {
                if (termina) {
                    break;
                } 
            } else {
                    perror("accept");
                    r = EXIT_FAILURE;
            }
        } 
        // Connessione accettata.
        if(spawn_thread(connfd)<0) {
            r = EXIT_FAILURE;
            break;
        }
    }
    unlink(SOCKNAME);
    return r;
}