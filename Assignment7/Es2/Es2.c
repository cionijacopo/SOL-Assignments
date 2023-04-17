#define _POSIX_C_SOURCE 200112L

#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>
#include<pthread.h>
#include<errno.h>
#include<assert.h>
#include<stdatomic.h>

#include "../includes/Utils.h"

// Questa è la lock, la chiamo forchetta per via del problema.
// Mi serviranno più lock, quindi dovrò inizializzarle con una init una per una
typedef pthread_mutex_t forchetta_t;
// Argomenti del thread. Sono più di uno.
typedef struct threadArgs {
    int thread_id;
    int N;
    forchetta_t *forks;
} threadArgs_t;

// Durata della cena, credo 2000 secondi
static long ITER = 2000;

// Funzione che esegue la sottrazione in modo atomico. 
// Mi serve per decrementare di uno.
static inline long atomicDec(long *v) {
    long *r;
    // Sottrae uno da v e lo restituisce su r
    r = atomic_fetch_sub(v, 1);
    return r;
}

void Mangia(unsigned int *seed) {
    long r = rand_r(seed) % 800000;
    // per perdere un po' di tempo si puo'
    // usare anche il seguente ciclo for
    // for(volatile long i=0;i<r; ++i);
    struct timespec t={0, r};
    nanosleep(&t,NULL);
}
void Pensa(unsigned int *seed) {
    long r = rand_r(seed) % 1000000;
    //for(volatile long i=0;i<r; ++i);
    struct timespec t={0, r};
    nanosleep(&t,NULL);
}

void *filosofo (void *arg) {
    int myid = ((threadArgs_t*)arg)->thread_id;
    int N = ((threadArgs_t*)arg)->N;
    int left = myid % N;
    int right = myid-1;
    forchetta_t* destra = &((threadArgs_t*)arg)->forks[right];
    forchetta_t* sinistra = &((threadArgs_t*)arg)->forks[left];
    // Genero un seed diverso per ogni filosofo.
    unsigned int seed = myid * time(NULL);
    int mangia = 0;
    int pensa = 0;
    for( ;; ) {
        if(atomicDec(&ITER) <= 0) break;

        Pensa(&seed);
        pensa++;

        if (left < right) {
	    LOCK(destra);
	    LOCK(sinistra);
	    mangia++;
	    Mangia(&seed);
	    UNLOCK(sinistra);
	    UNLOCK(destra);
	    
	} else {
	    LOCK(sinistra);
	    LOCK(destra);
	    mangia++;
	    Mangia(&seed);
	    UNLOCK(destra);
	    UNLOCK(sinistra);
	}
    }
    fprintf(stdout, "Filosofo %d:  ho mangiato %d volte e pensato %d volte\n", myid, mangia, pensa);
    fflush(stdout);
    return NULL;
}

int main (int argc, char *argv[]) {
    // Fisso un numero minimo per Nù
    int N = 5;
    if(argc>1) {
        N = atoi(argv[1]);
        if (N > 100) {
            fprintf(stderr, "N troppo grande, ridotto a 100.\n");
            N = 100;
        }
    }
    // Puntatore ai thread filosofi che vado a creare
    pthread_t *th;
    // Puntatore agli argomenti
    threadArgs_t *thARGS;
    // Puntatore ai mutex per la lock
    forchetta_t *forks;

    // Inizializzo lo spazio per i threads.
    // Sarebbe opportuno vare la macro per gestire l'errore della MALLOC
    // Qui non viene fatto.
    th = malloc(N*sizeof(pthread_t));
    thARGS = malloc(N*sizeof(threadArgs_t));
    // Creo il mio array dinamico di forks (mutex), 
    forks = malloc(N*sizeof(forchetta_t));
    if (!th || !thARGS || !forks) {
        fprintf(stderr, "Malloc fallita.\n");
        exit(EXIT_FAILURE);
    }

    // Inizzializzo il mutex, dato che ce ne sono più di uno non 
    // mi basta l'initializer in cima al programma
    for(int i = 0; i<N; i++) {
        if (pthread_mutex_init(&forks[i], NULL) != 0) {
            fprintf(stderr, "Pthread_t_mutex_init fallita. \n");
            exit(EXIT_FAILURE);
        }
    }
    /*
        Inizializzo anche gli argomenti. Per farlo setto l'id da 1 a N
        e attribuisco i mutex.
    */
   for(int i=0; i<N; i++) {
    thARGS[i].thread_id = (i+1);
    thARGS[i].N = N;
    thARGS[i].forks = forks;
   }

   /*
    Dopo aver inizializzato gli argomenti, creo i threads.
    In questo caso passo al thread il rispettivo argomento da 
    dare in pasto alla funzione.
   */
  for(int i = 0;i<N; i++) {
    if(pthread_create(&th[1], NULL, filosofo, &thARGS[i]) != 0) {
        fprintf(stderr, "Pthread_create fallita.\n");
        exit(EXIT_FAILURE);
    }
  }

  /*
  Dopo aver creato il thread, aspetto la sua terminazione con la join.
  */
 for (int i = 0; i < N; i++) {
    if(pthread_join(th[i], NULL) != 0) {
        fprintf(stderr, "Pthread_join fallita.\n");
    }
 }

 free(th);
 free(thARGS);
 free(forks);

 return 0;

}