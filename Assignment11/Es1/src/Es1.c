/*
    Created by Jacopo Cioni on 27/4/2023
    Esercizio 1 - Assignment 11
*/

#define _POSIX_C_SOURCE 200112L
#include<unistd.h>
#include<assert.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<ctype.h>
#include<signal.h>
#include<sys/select.h>

#include"../includes/utils.h"
#include"../includes/connection.h"
#include"../includes/threadpool.h"
#include"../includes/threadF.h"

/*
    Struttura contenente le informazioni da passare al signal handler thread
    Serve per la gestione dei segnali
*/
typedef struct {
    // Set di segnali da gestire
    sigset_t *set;
    // Descrittore di scrittura di una pipe senza nome
    int signal_pipe;
} sigHandler_t;

static void checkargs(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Utilizzare: %s <num_threads>\n", argv[0]);
        _exit(EXIT_FAILURE);
    }
    if((int)strtol(argv[1], NULL, 10) <= 0) {
        fprintf(stderr, "Errore: il numero di threads deve essere maggiore di zero.\n");
        fprintf(stderr, "Utilizzare: %s <num_threads>\n", argv[0]);
        _exit(EXIT_FAILURE);
    }
}

// Funzione eseguita dal signalHandler thread.
static void *sigHandler(void *args) {
    // Passo il set di segnali che si trova nell'argomento
    sigset_t *set = ((sigHandler_t*)args)->set;
    int fd_pipe = ((sigHandler_t*)args)->signal_pipe;

    for(;;) {
        int sig;
        int r = sigwait(set, &sig);
        if(r != 0) {
            errno = r;
            perror("FATAL ERROR sigwait");
            return NULL;
        }

        switch(sig) {
            case SIGINT:
            case SIGTERM:
            case SIGQUIT:
                close(fd_pipe);
                return NULL;
            default: ;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Controllo gli argomenti passati al programma
    checkargs(argc, argv);
    // Registro il numero di threads
    int num_threads = (int)strtol(argv[1], NULL, 10);

    // Comincio a settare la maschera per i segnali che mi interessano.
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);

    //Registro la maschera e blocco
    if(pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) {
        fprintf(stderr, "ERRORE FATALE: segnali.\n");
        unlink(SOCKNAME);
        return -1;
    }

    // Ignoro SIGPIPE per evitare di essere terminato da una scrittura su un socket.
    struct sigaction s;
    memset(&s, 0, sizeof(s));
    // Dico di ignorare
    s.sa_handler = SIG_IGN;
    // Passo all'handler che ignorerà SIGPIPE
    if((sigaction(SIGPIPE, &s, NULL)) == -1) {
        perror("sigaction");
        unlink(SOCKNAME);
        return -1;
    }

    /*
        Utilizzo una pipe come canale di comunicazione tra il signal handler thread ed il
        thread listener per notificare la terminazione.
    */
    int signal_pipe[2];
    if(pipe(signal_pipe) == -1) {
        perror("pipe");
        unlink(SOCKNAME);
        return -1;
    }

    pthread_t signalhandler_thread;
    sigHandler_t handlerArgs;
    handlerArgs.set = &mask;
    handlerArgs.signal_pipe = signal_pipe[1];

    // Creo il thread per la gestione dei segnali e gli passo la funzione sigHandler da utilizzare.
    if(pthread_create(&signalhandler_thread, NULL, sigHandler, &handlerArgs) != 0) {
        fprintf(stderr, "Errore nella creazione del signal handler thread.\n");
        unlink(SOCKNAME);
        return -1;
    }

    int listenfd;
    if((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        unlink(SOCKNAME);
        return -1;
    } 

    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);

    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("bind");
        unlink(SOCKNAME);
        return -1;
    }
    if(listen(listenfd, MAXBACKLOG) == -1) {
        perror("listen");
        unlink(SOCKNAME);
        return -1;
    }

    threadpool_t *pool = NULL;

    // Creazione del threadpool e gestione del threadpool.
    pool = createThreadPool(num_threads, num_threads);
    if(!pool) {
        fprintf(stderr, "ERRORE FATALE NELLA CREAZIONE DEL THREAD POOL\n");
        unlink(SOCKNAME);
        return -1;
    }

    // Set è il socket corrente
    // tmpset è il socket pronto per la lettura
    fd_set set, tmpset;
    //Inizializzo entrambi i set
    FD_ZERO(&set);
    FD_ZERO(&tmpset);
    // In particolare al al socket corrente inizializzo sia il descrittore del socket sia il descrittore della signal_pipe
    // Aggiungo il listener fd al master set
    FD_SET(listenfd, &set);
    // Aggiungo il descrittore di lettura della signal_pipe
    FD_SET(signal_pipe[0], &set);

    // Tengo traccia del file descriptor con id più grande
    int fdmax;
    if(listenfd > signal_pipe[0]) {
        fdmax = listenfd;
    } else {
        fdmax = signal_pipe[0];
    }

    volatile long termina = 0;
    while(!termina) {
        // Copio il set nella variabile temporanea per la select perchè la select è distruttiva
        tmpset = set;
        /*
            - Primo parametro: range di file descriptor da controllare
            - Secondo parametro: File descriptor che voglio controllare per la lettura
            - Ultimo argomento: Timeout
        */
        if(select(fdmax+1, &tmpset, NULL, NULL, NULL) == -1) {
            perror("select");
            unlink(SOCKNAME);
            return -1;
        }
        // Devo capire da quale file descriptor ho ricevuto la richiesta. Uno di questi sarà pronto per la lettura
        // Per capire quale è pronto devo controllarli tutti
        for(int i = 0; i <= fdmax; i++) {
            // Per ognuno controllo se è il set giusto
            if(FD_ISSET(i, &tmpset)) {
                long connfd;
                if(i == listenfd) {
                    // è il file descriptor del server socket
                    // C'è una nuova richiesta di connessione che posso accettare.
                    if (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL) == -1) {
                        perror("accept");
                        unlink(SOCKNAME);
                        return -1;
                    }
                    // Gestione degli argomenti
                    long *args = malloc(2*sizeof(long));
                    // Controllo che la malloc sia andata a buon fine.
                    if(!args) {
                        perror("FATAL ERROR malloc");
                        unlink("SOCKNAME");
                        return -1;
                    }
                    // Se la malloc è andata a buon fine setto gli argomenti
                    // Assegno il file descriptor della connessione
                    args[0] = connfd;
                    // Assegno l'indirizzo in cui è memorizzato termina
                    args[1] = (long)&termina;
                    // Aggiungo il task alla ThreadPool, passando la funzione e gli argomenti
                    // In questo caso gli argomenti sono il file descriptor della connessione e la variabile termina, sarà la funzione a gestire l'intera connessione
                    // del client.
                    int r = addToThreadPool(pool, threadF, (void*)args);
                    if (r == 0) {
                        // Task aggiunto con successo alla pool di threads
                        continue;
                    }
                    else if (r < 0) {
                        // Errore interno. Non è stato possibile aggiungere il task alla pool
                        fprintf(stderr, "FATAL ERROR, impossibile aggiungere task alla thread pool\n");
                    } else {
                        // In questo caso ritorna > 1, quindi tutti i threads sono occupati o la coda dei pendenti è piena
                        fprintf(stderr, "SERVER TOO BUSY\n");
                    }
                    // Terminata l'aggiunta, libero lo spazio degli argomenti e termino la connessione 
                    free(args);
                    close(connfd);
                    continue;
                }
                if (i == signal_pipe[0]) {
                    // Ho ricevuto un segnale di terminazione, esco ed inizio il protocollo di terminazione.
                    termina = 1;
                    break;
                }
            }
        }
    }

    // Distruggo la threadpool notificando che i thread dovranno terminare senza forzatura
    destroyThreadPool(pool, 0;)

    // Aspetto la terminazione del thread handler per i segnali
    pthread_join(signalhandler_thread, NULL);

    unlink(SOCKNAME);
    return 0;
}
