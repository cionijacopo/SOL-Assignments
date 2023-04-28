/*
    Created by Jacopo Cioni on 27/4/2023
    Implementazione dell'interfaccia Threadpool, funzioni per la creazione, la gestione e la distruzione.
*/

#include"../includes/threadpool.h"
#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<errno.h>
#include"../includes/utils.h"

//FUNZIONE ESEGUITA DAL THREAD WORKER CHE APPARTIENE AL POOL
static void *workerpool_thread(void *threadpool) {
    threadpool_t *pool = (threadpool_t *)threadpool;
    taskfun_t task;
    // pthread_self() restituisce l'id del thread che la chiama
    pthread_t self = pthread_self();
    int myid = -1;

    // Ricerco l'id del thread che sta eseguendo la funzione
    do {
        for(int i = 0; i < pool->numthreads; i++) {
            if(pthread_equal(pool->threads[i], self)) {
                myid = i;
                break;
            }
        }
    } while (myid < 0);

    LOCK_RETURN(&(pool->lock), NULL);
    for(;;) {
        // Rimango in attesa di un messaggio
        // Se non ci sono task in coda da eseguire && se non ho avviato l'uscita
        while((pool->count == 0) && (!pool->exiting)) {
            pthread_cond_wait(&(pool->cond), &(pool->lock));
        }
        if(pool->exiting > 1) {
            // Esco forzatamente, devo interrompere subito
            break;
        }
        if(pool->exiting == 1 && !pool->count) {
            // Esco, non ci sono task pendenti in coda
            break;
        }

        //Eseguo il nuovo task
        task.fun = pool->pending_queue[pool->head].fun;
        task.arg = pool->pending_queue[pool->head].arg;
        pool->head++;
        pool->count--;
        // Controllo la dimensione di pool->head, se ho raggigunto la dimensione massima la resetto a 0
        if (pool->head == abs(pool->queue_size)) {
            pool->head = 0;
        }
        // Aumento di 1 il numero di task in esecuzione
        pool->taskonthefly++;
        // rilascio la lock
        UNLOCK_RETURN(&(pool->lock), NULL);

        //Esecuzione della funzione
        (*(task.fun))(task.arg);

        // Riprendo la lock sulla pool per modificare il numero di task in esecuzione
        LOCK_RETURN(&(pool->lock), NULL);
        pool->taskonthefly--;
    }
    UNLOCK_RETURN(&(pool->lock), NULL);

    // Se sono uscito allora c'è stato un errore
    fprintf(stderr, "Thread %d in uscita.\n", myid);
    return NULL;
}

// FUNZIONE PER DEALLOCARE LE RISORSE DEDICATE ALLA THREADPOOL
static int freePoolResources(threadpool_t *pool) {
    if(pool->threads) {
        free(pool->threads);
        free(pool->pending_queue);

        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->cond));
    }
    free(pool);
    return 0;
}

// FUNZIONE PER LA DISTRUZIONE DEL THREADPOOL
int destroyThreadPool(threadpool_t *pool, int force) {
    if(pool == NULL || force < 0) {
        errno = EINVAL;
        return -1;
    }

    LOCK_RETURN(&(pool->lock), -1);

    // Se exiting è 1 aspetta che non ci siano più task in coda
    pool->exiting = 1 + force;

    if(pthread_cond_broadcast(&(pool->cond)) != 0) {
        UNLOCK_RETURN(&(pool->lock), -1);
        errno = EFAULT;
        return -1;
    }
    UNLOCK_RETURN(&(pool->lock), -1);

    for (int i = 0; i < pool->numthreads; i++) {
        if (pthread_join(pool->threads[i], NULL) != 0) {
            errno = EFAULT;
            UNLOCK_RETURN(&(pool->lock), -1);
            return -1;
        }
    }
    freePoolResources(pool);
    return 0;
}

// FUNZIONE PER LA CREAZIONE DI UN THREADPOOL
threadpool_t *createThreadPool(int numthreads, int pending_size) {
    if(numthreads <= 0 || pending_size < 0) {
        // EINVAL = invalid argument
        errno = EINVAL;
        return NULL;
    }

    threadpool_t *pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    if (pool == NULL) {
        return NULL;
    }
    // Threadpool allocata con successo
    // Setto le condizioni iniziali
    pool->numthreads = 0;
    pool->taskonthefly = 0;
    if(pending_size == 0) {
        pool->queue_size = -1;
    } else {
        pool->queue_size = pending_size;
    }
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    pool->exiting = 0;

    //Alloco lo spazio per i thread worker 
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t)*numthreads);
    if (pool->threads == NULL) {
        // Libero la memoria appena allocata per la pool e ritorno NULL
        free(pool);
        return NULL;
    }

    //Alloco lo spazio per la pending queue, ovvero per i task in attesa di essere processati.
    pool->pending_queue = (taskfun_t*)malloc(sizeof(taskfun_t) * abs(pool->queue_size));
    if(pool->pending_queue == NULL) {
        // Libero la memoria appena allocata per la pool e per il vettore di threads
        free(pool->threads);
        free(pool);
        return NULL;
    }

    // Inizializzo adesso i sistemi di lock con i mutex essendo l'accesso alla coda di tasks condivisa
    if( (pthread_mutex_init(&(pool->lock), NULL) != 0) || (pthread_cond_init(&(pool->cond), NULL) != 0) ) {
        // Libero la memoria appena allocata per la pool, per il vettore di threads e per la lista dei task pendenti
        free(pool->threads);
        free(pool->pending_queue);
        free(pool);
        return NULL;
    }

    // Adesso posso cominicare a creare i singoli threads.
    for(int i = 0; i < numthreads; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, workerpool_thread, (void*)pool) != 0) {
            // FATAL ERROR, libero tutta la memoria acquisita fino ad ora e forzo l'uscita dei threads.
            destroyThreadPool(pool, 1);
            errno = EFAULT;
            return NULL;
        }
        pool->numthreads++;
    }
    return pool;
}

// FUNZIONE CHE AGGIUNGE UN TASK AL POOL, SE CI SONO THREAD LIBERI IL TASK VIENE ASSEGNATO AD UNO DI QUESTI; ALTRIMENTI VIENE MESSO IN CODA.
int addToThreadPool(threadpool_t *pool, void (*f)(void *), void *arg) {
    if(pool == NULL || f == NULL) {
        errno = EINVAL;
        return -1;
    }
    // Prendo la lock
    LOCK_RETURN(&(pool->lock), -1);
    int queue_size = abs(pool->queue_size);
    // Se pool->queue_size == -1 allora not_pending vale 1
    int not_pending = (pool->queue_size == -1);

    // Coda piena o in fase di attesa.
    if(pool->count >= queue_size || pool->exiting) {
        UNLOCK_RETURN(&(pool->lock), -1);
        // Esco con valore "CODA PIENA"
        return 1;
    }
    //Controllo se il numero di task attualmente in esecuzione è maggiore o uguale al numero di threads 
    if(pool->taskonthefly >= pool->numthreads) {
        // Controllo se non si vogliono gestire task pendenti
        if(not_pending) {
            // Tutti i threads sono occupati e non si gestiscono task pendenti
            assert(pool->count = 0);
            UNLOCK_RETURN(&(pool->lock), -1);
            // Esco con valore "CODA PIENA"
            return 1;
        }
    }

    pool->pending_queue[pool->tail].fun = f;
    pool->pending_queue[pool->tail].arg = arg;
    pool->count++;
    pool->tail++;

    if(pool->tail >= queue_size) {
        pool->tail = 0;
    }
    int r;
    if((r=pthread_cond_signal(&(pool->cond))) != 0) {
        UNLOCK_RETURN(&(pool->lock), -1);
        errno = r;
        return -1;
    }
    UNLOCK_RETURN(&(pool->lock), -1);

    return 0;
}

// FUNZIONE ESEGUITA DAL THREAD SPAWNATO NON APPARTENENTE AL POOL
static void *proxy_thread(void *arg) {
    taskfun_t *task = (taskfun_t *)arg;
    // Eseguo la funzione.
    (*(task->fun))(task->arg);

    free(task);
    return NULL;
}

//FUNZIONE CHE FA LO SPAWN DI UN THREAD IN MODALITÀ DETACHED
// QUESTO THREAD NON APPARTIENE AL THREADPOOL
int spawnThread(void (*f)(void*), void *arg) {
    if (f == NULL) {
        errno = EINVAL;
        return -1;
    }

    //Genero il task allocando la memoria necessaria. 
    // Lo spazio verrà poi liberato dal proxy thread
    taskfun_t *task = malloc(sizeof(taskfun_t));
    if(!task) {
        // Errore malloc
        return -1;
    }
    task->fun = f;
    task->arg = arg;

    pthread_t thread;
    pthread_attr_t attr;
    if(pthread_attr_init(&attr) != 0) {
        return -1;
    }
    if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
        return -1;
    }
    if(pthread_create(&thread, &attr, proxy_thread, (void*)task) != 0) {
        free(task);
        errno = EFAULT;
        return -1;
    }
    return 0;
}