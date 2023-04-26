/*
    Created by Jacopo Cioni 
    Raccolta di Utils 26/4/2023
*/

#if !defined(_UTILS_H)
#define _UTILS_H

// stdarg.h --> Header che consente alle funzioni di accettare un numero indefinito di argomenti
#include<stdarg.h>
// sys/types.h --> Header che definisce una raccolta di simboli e strutture typedef
#include<sys/types.h>
// sys/stat.h --> Header che contiene costrutti che facilitano la raccolta di informazioni sugli attributi dei file
#include<sys/stat.h>
// fcntl.h --> Header che definisce le opzioni di controllo file
#include<fcntl.h>
// unistd.h --> Header che consente l'accesso alle API dello standard POSIX (fork, pipe, ....)
#include<unistd.h>
// stdlib.h --> Header che contiene funzioni e costanti di utilità generale (accesso memoria, controllo processi, tipi di dato)
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
// pthread.h --> Header che contiene dichiarazioni di funzioni e mappature per le interfacce di threading.
#include<pthread.h>
// errno.h --> Header che contiene definizioni di macro per la gestione delle situazioni di errore.
#include<errno.h>

#if !defined(BUFSIZE) 
#define BUFSIZE 256
#endif

#if !defined(EXTRA_LEN_PRINT_ERROR)
#define EXTRA_LEN_PRINT_ERROR 512
#endif

// Check chiamata di sistema con stampa ed uscita.
#define SYSCALL_EXIT(name, r, sc, str, ...)	    \
    if ((r=sc) == -1) {				            \
	    perror(#name);				            \
	    int errno_copy = errno;			        \
	    print_error(str, __VA_ARGS__);		    \
	    exit(errno_copy);			            \
    }

// Check chiamata di sistema con SOLO stampa
#define SYSCALL_PRINT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				            \
	    perror(#name);				            \
	    int errno_copy = errno;			        \
	    print_error(str, __VA_ARGS__);		    \
	    errno = errno_copy;			            \
    }

// Check chiamata di sistema con stampa e ritorno di errore
#define SYSCALL_RETURN(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				            \
	    perror(#name);				            \
	    int errno_copy = errno;			        \
	    print_error(str, __VA_ARGS__);		    \
	    errno = errno_copy;			            \
	    return r;                               \
    }

// Check equality con uscita
#define CHECK_EQ_EXIT(name, X, val, str, ...)	\
    if ((X)==val) {				                \
        perror(#name);				            \
	    int errno_copy = errno;			        \
	    print_error(str, __VA_ARGS__);		    \
	    exit(errno_copy);			            \
    }

// Check not equality con uscita
#define CHECK_NEQ_EXIT(name, X, val, str, ...)	\
    if ((X)!=val) {				                \
        perror(#name);				            \
	    int errno_copy = errno;			        \
	    print_error(str, __VA_ARGS__);		    \
	    exit(errno_copy);			            \
    }

/*
    Macro per prendere la lock.
    - pthread_mutex_lock prende la lock e restituisce 0 se tutto
    va bene. È importantissima la gestione di questi tipi di errore.
    - pthread_exit serve per terminare un thread.Termina il thread e
    restituisce lo status alla join.
*/
#define LOCK(l)                                     \
    if(pthread_mutex_lock(l) != 0) {                \
        fprintf(stderr, "ERRORE FATALE lock\n");    \
        pthread_exit((void*) EXIT_FAILURE);         \
    }

/*
    Macro per prendere la lock.
    - pthread_mutex_lock prende la lock e restituisce 0 se tutto
    va bene. È importantissima la gestione di questi tipi di errore.
*/
#define LOCK_RETURN(l, r)                           \
    if(pthread_mutex_lock(l) != 0) {                \
        fprintf(stderr, "ERRORE FATALE lock\n");    \
        return r;                                   \
    }

/*
    Macro per rilasciare la lock.
    - pthread_mutex_unlock rilascia la lock e restituisce 0 se tutto
    va bene. È importantissima la gestione di questi tipi di errore.
    - pthread_exit serve per terminare un thread. Termina il thread e
    restituisce lo status alla join.
*/
#define UNLOCK(l)                                       \
    if(pthread_mutex_unlock(l) != 0) {                  \
        fprintf(stderr, "ERRORE FATALE unlock\n");      \
        pthread_exit((void*) EXIT_FAILURE);             \
    }

/*
    Macro per rilasciare la lock.
    - pthread_mutex_unlock rilascia la lock e restituisce 0 se tutto
    va bene. È importantissima la gestione di questi tipi di errore.
*/
#define UNLOCK_RETURN(l, r)                             \
    if(pthread_mutex_unlock(l) != 0) {                  \
        fprintf(stderr, "ERRORE FATALE unlock\n");      \
        return r;                                       \
    }

/*
    Macro per mettere in attesa un thread su una determinata condizione.
    - pthread_cond_wait mette in attesa un thread fino a quando non si 
    verifica una determinata condizione c. Nel caso di fallimento viene 
    gestito l'errore.
    - pthread_exit termina il thread.
*/
#define WAIT(c, l)                                      \
    if(pthread_cond_wait(c, l) != 0){                   \
        fprintf(stderr, "ERRORE FATALE wait\n");        \
        pthread_exit((void*) EXIT_FAILURE);             \
    }

/*
    Macro per mettere in attesa un thread su una determinata condizione
    per un tempo specifico.
    - pthread_cond_timedwait mette in attesa un thread fino a quando
    non si verifica una determinata condizione per una quantità di 
    tempo limitata. Nel caso di fallimento viene gestito l'errore.
    Restituisce 0 se tutto va bene.
*/
#define TWAIT(c, l, t)                                                  \
    int r = 0;                                                          \
    if(r =pthread_cond_timedwait(c, l, t) != 0 && r != ETIMEDOUT) {     \
        fprintf(stderr, "ERRORE FATALE twait\n");                       \
        pthread_exit((void*) EXIT_FAILURE);                             \
    }

/*
    Macro per segnalare un evento e risvegliare un thread.
    - pthread_cond_signal effettua la signal per risvegliare un thread 
    su quella particolare condizione. Restituisce 0 se tutto va bene.
*/
#define SIGNAL(c)                                    \
    if(pthread_cond_signal(c) != 0) {                \
        fprintf(stderr, "ERRORE FATALE signal\n");   \
        pthread_exit((void*) EXIT_FAILURE);          \
    }

/*
    Macro per segnalare un evento e risvegliare i thread.
    - pthread_cond_broadcast effettua la signal per risvegliare tutti i thread 
    su quella particolare condizione che poi dovrenno competere per
    riusire a prendere la lock. Restituisce 0 se tutto va bene. È 
    l'equivalente del signalall().
*/
#define BCAST(c)                                        \
    if(pthread_cond_broadcast(c) != 0){                 \
        fprintf(stderr, "ERRORE FATALE broadcast\n");   \
        pthread_exit((void*) EXIT_FAILURE);             \
    }

/*
    Funzione che prova a prendere la lock. Se ci riesce torna 0 e prende
    la lock, altrimenti ritorna un codice di errore.
*/
static inline int TRYLOCK(pthread_mutex_t* l) {
  int r=0;		
  if ((r=pthread_mutex_trylock(l))!=0 && r!=EBUSY) {		    
    fprintf(stderr, "ERRORE FATALE unlock\n");		    
    pthread_exit((void*)EXIT_FAILURE);			    
  }								    
  return r;	
}

// Procedura di utilita' per la stampa degli errori
static inline void print_error(const char * str, ...) {
    const char err[]="ERROR: ";
    va_list argp;
    char * p=(char *)malloc(strlen(str)+strlen(err)+EXTRA_LEN_PRINT_ERROR);
    if (!p) {
	perror("malloc");
        fprintf(stderr,"FATAL ERROR nella funzione 'print_error'\n");
        return;
    }
    strcpy(p,err);
    strcpy(p+strlen(err), str);
    va_start(argp, str);
    vfprintf(stderr, p, argp);
    va_end(argp);
    free(p);
}

/* 
  Controlla se la stringa passata come primo argomento e' un numero.
  0 ok  1 non e' un numbero   2 overflow/underflow
 */
static inline int isNumber(const char* s, long* n) {
  if (s==NULL) return 1;
  if (strlen(s)==0) return 1;
  char* e = NULL;
  errno=0;
  long val = strtol(s, &e, 10);
  if (errno == ERANGE) return 2;    // overflow/underflow
  if (e != NULL && *e == (char)0) {
    *n = val;
    return 0;   // successo 
  }
  return 1;   // non e' un numero
}


#endif