/*
    Created by Jacopo Ciono on 20/4/2023
    Esercizio 1 - Assignment 9
    Implementazione del server. 
*/

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include "../includes/Utils.h"
#include "../includes/Connection.h"

// A cosa serve ?
#define CHECKNULL(r, c, e) CHECK_EQ_EXIT(e, (r=c), NULL, e, "")

//Definisco la struttura del messaggio
typedef struct msg
{
    int len;
    char *str;
} msg_t;

// Operazione di pulizia per cancellare il socket file se già esiste
void cleanup() {
    // Unlink rimuove un nome dal filesystem.
    unlink(SOCKNAME);
}

int cmd(const char str[], char *buf) {
    int to[2];
    int from[2];

    int notused;
    /*
        Creo le pipe con la system call pipe(), passando come argomento
        il vettore che fa da pipe. La funzione ritorna due descrittori
        di file attraverso l'argomento, quello [0] è aperto in lettura e
        quello [1] è aperto in scrittura.
    */ 
   SYSCALL_EXIT("pipe", notused, pipe(to), "pipe1", "");
   SYSCALL_EXIT("pipe", notused, pipe(from), "pipe2", "");

   // Faccio la fork, separo il processo in padre e figlio.
   // Se la fork da 0 sono il figlio, se da >0 sono il padre
   if(fork() == 0) {
    // Sono il figlio
    /*
        Chiudo i descrittori non utilizzati. In questo caso chiudo:
        - chiudo il descrittore di scrittura di "to" (quindi [1])
        - chiudo il descrittore di lettura di "from" (quindi [0])
    */ 
   SYSCALL_EXIT("close", notused, close(to[1]), "close", "");
   SYSCALL_EXIT("close", notused, close(from[0]), "close", "");

    /* 
        La dup2 serve per associare lo stdout o lo stdin di un programma,
        rispettivamente, al lato in scrittura o a quello in lettura di una
        pipa. Duplica il descrittore del primo parametro nel descrittore del
        secondo parametro. 
        Vale che:
        - lo stdin è identificato con 0
        - lo stdout è identificato con 1 
        - lo stderr è identificato con 2
    */
   // Associo il descrittore di lettura allo stdin.
   SYSCALL_EXIT("dup2", notused, dup2(to[0], 0), "dup child (1)", "");
   // Associo il descrittore di scrittura allo stdout
   SYSCALL_EXIT("dup2", notused, dup2(from[1], 1), "dup2 child (2)", "");
   // Associo il descrittore di scrittura allo stderr
   SYSCALL_EXIT("dup2", notused, dup2(from[1], 2), "dup2 child (2)", "");
   
   // Richiamo adesso la execl() Per l'esecuzione del comando "bc" 
   // (calcolatrice testuale). 
   execl("/usr/bin/bc", "bc", "-l", NULL);
   return -1;
   } 
   // Sono il padre (?)
   // Chiudo la lettura di to, chiudo la scrittura di from
   SYSCALL_EXIT("close", notused, close(to[0]), "close","");
   SYSCALL_EXIT("close", notused, close(from[1]), "close","");
   int n;
   SYSCALL_EXIT("write", n, write(to[1], (char*)str, strlen(str)), "written", "");
   SYSCALL_EXIT("read",  n, read(from[0], buf, BUFSIZE), "readn","");  // leggo il risultato o l'errore
   SYSCALL_EXIT("close", notused, close(to[1]), "close","");  // si chiude lo standard input di bc cosi' da farlo terminare
   SYSCALL_EXIT("wait", notused, wait(NULL), "wait","");
   return n;
}

int main(int argc, char *argv[]) {
    // Devo cancellare il socket file se già esiste
    // Per farlo faccio un'operazione di cleanup()
    cleanup();
    // Se qualcosa va storto registro la funzione perchè sia chiamata
    // alla fine del programma per segnalare l'errore
    atexit(cleanup);

    int listenfd;

    // Creazione del socket mediante le system call.
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket", "");

    // Setto l'indirizzo del socket
    struct socketaddr_un serv_addr;
    // memset copia il secondo argomento nei primi n caratteri del
    // primo argomento.
    memset (&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncopy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    int notused;
    //Faccio la bind e assegno l'indirizzo al socket. Devo passargi
    // listenfd ricevuto con la socket prima
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "bind", "");

    //Faccio la listen e setto il socket in modalità passiva. 
    // Setto anche un numero di connessioni massime che il kernel tiene
    // in coda.
    SYSCALL_EXIT("listen", notused, listen(listenfd, 1), "listen", "");

    int connfd, n;
    do {
        // Faccio l'accept per accettare la connessione.
        SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*) NULL, NULL ), "accept", "");
        msg_t str;
        CHECKNULL(str.str, malloc(BUFSIZE), "malloc");

        while(1) {
            char buffer[BUFSIZE];
            memset(str.str, '\0', BUFSIZE);
            // La readn legge n bytes da un descriptor
            // Leggo prima la dimensione del messaggio (stringa) e poi
            // leggo il messaggio
            SYSCALL_EXIT("readn", n, readn(connfd, &str.len, sizeof(int)), "readn1", "");
            SYSCALL_EXIT("readn", n, readn(connfd, str.str, str.len), "readn2", "");

            // La system call memorizza su n il numero di bytes letti
            if(n == 0) {
                break;
            }
            memset(buffer, '\0', BUFSIZE);
            if ((n = cmd(str.str, buffer)) < 0) {
                fprintf(stderr, "Errore nell'esecuzione del comando.\n");
                break;
            }
            buffer[n] = '\0';

            // Invio la risposta
            // Prima comunico il numero di byte da scrivere
            SYSCALL_EXIT("writen", notused, writen(connfd, &n, sizeof(int)), "writen1", "");
            // Poi scrivo tutti i byte e svuoto il buffer.
            SYSCALL_EXIT("writen", notused, writen(connfd, buffer, n), "writen2", "");
        }
        // chiusura del socket
        close(connfd);
        printf("Connessione eseguita.\n");
        if(str.str) {
            free(str.str);
        }
    } while (1);
    return 0;
}