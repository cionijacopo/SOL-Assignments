/* 
Created by Jacopo Cioni on 30/3/2023
Esercizio 4 - Assignment 2
*/

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

int rand_r(unsigned int *seddp);

#define N 10000
#define K1 0
#define K2 10

int isNumber (const char *s, long *n) {
    if (s == NULL) {
        return 1;
    }
    if (strlen(s) == 0) {
        return 1;
    }
    char *e = NULL;
    errno = 0;
    long val = strtol(s, &e, 10);
    if (errno == ERANGE) {
        return 2;
    }
    if (e != NULL & *e == (char)0) {
        *n = val;
        return 0;
    }
    return 1;
}

int main (int argc, char *argv[]) {
    long k1 = K1;
    long k2 = K2;
    long n = N;
    // Controllo che ci siano valori
    if(argc > 1) {
        // Se ci sono valori, non può essere un numero diverso da 4
        if (argc != 4) {
            printf("Utilizzare: %s [K1 K2 N]\n", argv[0]);
            return -1;
        }
        // Se sono 4 argomenti, controllo che siano quelli giusti
        if (isNumber(argv[1], &k1) != 0) {
            printf("l'argomento %s non e' valido\n", argv[1]);
            return -1;
        }
        if (isNumber(argv[2], &k2) != 0) {
            printf("l'argomento %s non e' valido\n", argv[2]);
            return -1;
        }
        if (isNumber(argv[3], &n) != 0) {
            printf("l'argomento %s non e' valido\n", argv[3]);
            return -1;
        }
        if (k2<=k1) {
            printf("%ld deve essere maggiore di %ld\n", k2,k1);
            return -1;
        }	
    }

    // Definisco il seme per i numeri random
    // In questo caso utilizzo la funzione time
    unsigned int seed = time(NULL);
    const long k = k2-k1;

    long x[k];
    // Inizializzo il vettore
    for(long i = 0; i<k; i++) {
        x[i] = 0;
    }
    for(long i = 0; i<n; i++) {
        long r = k1 + rand_r(&seed)%k;
        x[r%k]++;
    }
    printf("Occorrenze di: \n");
    for(long i=0;i<k;++i){
        printf("%ld  : %5.2f%% \n", i+k1, (x[i]/(float)n)*100);
    }
    printf("\n");
    return 0;
}