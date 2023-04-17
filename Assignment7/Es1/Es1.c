#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

#include"../includes/Utils.h"

// Inizializzazione di un mutex globale. Serve per prendere la lock.
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// Inizializzazione di una condition globale. Serve per la wait.
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
 
// Buffer di una posizione globale per tutto il programma
//In questo caso è una risorsa condivisa.
static int buffer;
// Flag che indica se il buffer è pieno
// 1 - è vuoto, 0 - è pieno
static int empty = 1;

// Funzione che do in pasto al thread produttore.
// Deve essere un puntatore a funzione. (IMPORTANTE)
// <notused> in questo caso prenderà NULL, lo metto perchè non ho parametri
void *producer(void *notused) {
    // Faccio questo gioco 1000 volte mettendo i nel buffer.
    for (int i = 0; i<1000; i++) {
        // Devo prendere la lock prima di accedere alla variabile condivisa
        LOCK(&mutex);
        // Se empty vale 0 (buffer pieno) allora devo aspettare.
        while (empty == 0) {
            WAIT(&cond, &mutex);
        }
        buffer = i;
        // Adesso il buffer è pieno.
        empty = 0;
        // Segnalo che il buffer è pieno al consumatore
        // Rilascio la lock
        SIGNAL(&cond);
        UNLOCK(&mutex);
        
    }
    // Produco un valore speciale per fare terminare il consumatore
    printf("Produttore terminato.\n");
    // riprendo la lock per inserire nel buffer il valore di terminazione
    // per il consumatore.
    LOCK(&mutex);
    while(empty == 0) {
        WAIT(&cond, &mutex);
    }

    // Valore speciale
    buffer = -1;
    empty = 0;
    SIGNAL(&cond);
    UNLOCK(&mutex);

    return NULL;
}

// Funzione che do in pasto al thread produttore.
// Deve essere un puntatore a funzione. (IMPORTANTE)
// <notused> in questo caso prenderà NULL, lo metto perchè non ho parametri
void *consumer(void *notused) {
    int val = 0;
    while (val >= 0) {
        LOCK(&mutex);
        while(empty == 1) {
            WAIT(&cond, &mutex);
        }
        val = buffer;
        empty = 1;
        SIGNAL(&cond);
        UNLOCK(&mutex);

        printf("Consumato il valore: %d\n", val);
    }

    printf("Consumatore terminato.\n");
    return NULL;
}

int main() {
    pthread_t consumatore, produttore;

    /*
        La funzione pthread_create crea il thread e restituisce 0 se va
        tutto bene. Parametri alla funzione:
        - Il thread da inizializzare
        - Le varie option (nel nostro caso null)
        - La funzione da dare in pasto dal thread
        - Parametri
    */
    if(pthread_create(&consumatore, NULL, consumer, NULL) != 0) {
        fprintf(stderr, "Errore nella creazione del thread (consumatore)\n");
        return(EXIT_FAILURE);
    }
    /*
        La funzione pthread_create crea il thread e restituisce 0 se va
        tutto bene. Parametri alla funzione:
        - Il thread da inizializzare
        - Le varie option (nel nostro caso null)
        - La funzione da dare in pasto dal thread
        - Parametri
    */
    if(pthread_create(&produttore, NULL, producer, NULL) != 0) {
        fprintf(stderr, "Errore nella creazione del thread (produttore)\n");
        return(EXIT_FAILURE);
    }

    /*
        La funzione pthread_join sospende il processo che invoca il thread
        fino a quando il thread identificato termina. In questo caso 
        sospendo il processo main. Restituisce 0 se tutto va bene.
        Parametri: 
        - Il thread da attendere
        - Lo status di terminazione (se non lo voglio metto NULL)
    */
   if(pthread_join(produttore, NULL) != 0) {
    fprintf(stderr, "Errore nell'attesa del thread (produttore)\n");
    return(EXIT_FAILURE);
   }
   /*
        La funzione pthread_join sospende il processo che invoca il thread
        fino a quando il thread identificato termina. In questo caso 
        sospendo il processo main. Restituisce 0 se tutto va bene.
        Parametri: 
        - Il thread da attendere
        - Lo status di terminazione (se non lo voglio metto NULL)
    */
   if(pthread_join(consumatore, NULL) != 0) {
    fprintf(stderr, "Errore nell'attesa del thread (consumatore)\n");
    return(EXIT_FAILURE);
   }

    return 0;
}