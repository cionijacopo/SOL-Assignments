#if !defined (_UTILS_H)
#define _UTILS_H

/*
In questa utils ci sono parti che non servono necessariamente per svolgere
questo esercizio. Mano a mano che andiamo avanti con gli assignments lo aggiorniamo
in modo da averlo completo per l'utilizzo del progetto.
Possiamo riutilizzarlo (magari organizzato meglio, credo) per il progetto.
*/
#include<stdarg.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<errno.h>

#if !defined(BUFSIZE)
#define BUFSIZE 256
#endif

#if !defined(EXTRA_LEN_PRINT_ERROR)
#define EXTRA_LEN_PRINT_ERROR   512
#endif

#define SYSCALL_EXIT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

#define SYSCALL_PRINT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	errno = errno_copy;			\
    }

#define CHECK_EQ_EXIT(name, X, val, str, ...)	\
    if ((X)==val) {				\
        perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

#define CHECK_NEQ_EXIT(name, X, val, str, ...)	\
    if ((X)!=val) {				\
        perror(#name);				\
	int errno_copy = errno;			\
	print_error(str, __VA_ARGS__);		\
	exit(errno_copy);			\
    }

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

// Parte nuova con le macro per la gestione dei threads.
/*
    - pthread_mutex_lock prende la lock e restituisce 0 se tutto
    va bene. È importantissima la gestione di questi tipi di errore.
    - pthread_exit serve per terminare un thread. Viene sempre scritta
    in questo modo (credo). Termina il thread e restituisce lo status 
    alla join.
*/
#define LOCK(l)                                     \
    if(pthread_mutex_lock(l) != 0) {                \
        fprintf(stderr, "ERRORE FATALE lock\n");    \
        pthread_exit((void*) EXIT_FAILURE);         \
    }

/*
    - pthread_mutex_unlock rilascia la lock e restituisce 0 se tutto
    va bene. È importantissima la gestione di questi tipi di errore.
    - pthread_exit serve per terminare un thread. Viene sempre scritta
    in questo modo (credo). Termina il thread e restituisce lo status 
    alla join.
*/
#define UNLOCK(l)                                       \
    if(pthread_mutex_unlock(l) != 0) {                  \
        fprintf(stderr, "ERRORE FATALE unlock\n");      \
        pthread_exit((void*) EXIT_FAILURE);             \
    }

/*
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
    - pthread_cond_signal effettua la signal per risvegliare un thread 
    su quella particolare condizione. Restituisce 0 se tutto va bene.
*/
#define SIGNAL(c)                                    \
    if(pthread_cond_signal(c) != 0) {                \
        fprintf(stderr, "ERRORE FATALE signal\n");   \
        pthread_exit((void*) EXIT_FAILURE);          \
    }

/*
    - pthread_cond_broadcast effettua la signal per risvegliare tutti i thread 
    su quella particolare condizione che poi dovrenno competere per
    riusire a prendere la lock. Restituisce 0 se tutto va bene. È 
    l'equivalente del signalall() in java.
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

#endif