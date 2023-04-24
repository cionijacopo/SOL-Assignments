/*
    Created by Jacopo Cioni on 24/4/2023
    Esercizio 1 - Assignment 10
*/

#define _POSIX_C_SOURCE 200112L
#include<stdio.h>
#include<signal.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<sys/select.h>
#include<sys/time.h>
#include<sys/types.h>

#include"../includes/Utils.h"

static volatile sig_atomic_t sign_int_counter = -1;
static volatile sig_atomic_t sign_stp_counter = -1;
static volatile sig_atomic_t sign_stp_flag = 0;

static void sighandler(int segnale) {
    switch(segnale) {
        case SIGINT: {
            if (sign_int_counter == -1) {
                sign_int_counter = 0;
            }
            sign_int_counter++;
        } break;
        case SIGTSTP: {
            sign_stp_flag = 1;
            if (sign_stp_counter == -1) {
                sign_stp_counter = 0;
            }
            sign_stp_counter++;
        } break;
        case SIGALRM: {
            return;
        } break;
        default: {
            // Causa la terminazione del processo in modo anomalo. 
            // Succede quando l'handler non ritorna nulla, in tal caso abort
            // passa l'errore ad atexit che stamperà alla chiusura del programma.
            abort();
        }
    }
}

int main(void) {
    sigset_t mask, oldmask;
    // Azzero la maschera puntata dall'argomento
    sigemptyset(&mask);
    // Setto a 1 la posizione del segnale che mi interessa
    // SIGINT -> richiesta di interruzione da tastiera (Ctrl+C)
    sigaddset(&mask, SIGINT);
    // SIGTSTP -> Richiesta di sospensione da tastiera (Ctrl+Z)
    sigaddset(&mask, SIGTSTP);
    // sigprocmask cambia/setta i segnali del thread che chiama questa funzione
    // SIG_BLOCK suggerisce che i segnali SIGINT e SIGTSTP siano bloccati
    // fino a quando non ho concluso l'installazione degli handler.
    // Conservo la vecchia maschera in "oldmask."
    if(sigprocmask(SIG_BLOCK, &mask, &oldmask) == -1) {
        perror("sigprocmask");
        return EXIT_FAILURE;
    } 

    // Installo un unico signal handler per tutti i segnali che mi interessano
    struct sigaction sa;
    // Resetto la struttura e la preparo inserendo tutti 0
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sighandler;

    // Preparo la maschera per l'handler.
    sigset_t handler_mask;
    sigemptyset(&handler_mask);
    sigaddset(&handler_mask, SIGINT);
    sigaddset(&handler_mask, SIGTSTP);
    sigaddset(&handler_mask, SIGALRM);
    // Handler eseguirà con SIGINT E SIGTSTP segnati
    sa.sa_mask = handler_mask;
    // La funzione sigaction() serve per permetter al processo che la
    // chiama di esaminare e gestire l'azione associata ad uno specifico
    // segnale. Il secondo parametro è la struttura che definisce il nuovo
    // trattamento del segnale catturato (primo parametro).
    // sigaction ritorna -1 in caso di errore.
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
    }
    if(sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("sigaction SIGSTOP");
    }
    if(sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction SIGALRM");
    }

    if(system("clear")<0) {
        perror("system");
    }

    while(1) {
        /*
            This function replaces the process’s signal mask with set
            and then suspends the process until a signal is delivered
            whose action is either to terminate the process or invoke 
            a signal handling function. In other words, the program is
            effectively suspended until one of the signals that is not
            a member of set arrives.
        */
       if (sigsuspend(&oldmask) == -1 && errno != EINTR) {
        perror("sigsuspend");
        return(EXIT_FAILURE);
       }

       // Qui i segnali sono nuovamente bloccati 
       if(sign_stp_flag) {
        printf("Ricevuti %d SIGINT\n", (sign_int_counter<0)?0:sign_int_counter);
        sign_stp_flag = 0;
        sign_int_counter = 0;
       }
       if (sign_stp_counter == 3) {
        sign_stp_counter = 0;
        printf("Per continuare premere invio, altrimenti verrai terminato entro 10s.\n");
        fflush(stdout);
        alarm(10);
        fd_set set, tmpset;
        FD_ZERO(&set);
        FD_SET(0, &set);
        int cnt = 10;
        char c = '\0';
        do {
	      // questa parte non era prevista nell'esercizio......
	      // si usa una select per implementare il countdown
	      // nella stampa
	      
		struct timeval tm={1,0};
		tmpset = set;
		int r=select(0+1, &tmpset, NULL, NULL, &tm);
		switch(r) {
		case -1: {
		    if (errno == EINTR) {
			//printf("\033[2J"); // Clear screen (man console_code)
			printf("Terminato!\n");
			return 0;
		    }
		    perror("select");
		    exit(EXIT_FAILURE);
		} break;
		case 0: // timeout
		    --cnt;
		    printf("\033[1A");  // Move cursor up 1 row  (man console_code)
		    printf("Per continuare premere invio, altrimenti verrai terminato entro %d secondi \n",cnt);
		    fflush(stdout);
		    continue;
		default:;
		}

		if (read(0, &c, sizeof(char)) == -1 && errno != EINTR) {
		    perror("read");
		    return EXIT_FAILURE;
		}
		alarm(0); // resetto l'allarme..... potrei non farcela
		if (system("clear")<0) {
		  perror("system");
		}
	    } while(c=='\0' && cnt>0);
       }
    }

    return 0;

}

