/*
Created by Jacopo Cioni on 1/4/2023
Esercizio 4 - Assignment 3
*/

#include<stdio.h>
#include<stdlib.h>

#if !defined(INIT_VALUE)

#error("Compilare con il flag -DINIT_VALUE=n");

#endif

//Versione NON rientrante
int somma(int x) {
    static int s = INIT_VALUE;
    s = s+x;
    return s;
}

//Versione rientrante
int somma_r(int x, int *s) {
    *s = *s +x;
    return *s;
}


int main (int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Utilizzare: %s N\n", argv[0]);
        return -1;
    }
    int N = strtol(argv[1], NULL, 10);
    if(N<=0||N>10) {
        fprintf(stderr, "N deve esssere compreso tra 1 e 10.\n");
        return -1;
    }
    // Variabile utilizzata per la versione rientrante.
    int s = INIT_VALUE;

    for(int i=0;i<N;i++) {
        int val;
        fprintf(stdout, "Inserisci un numero (%d rimasti): ", N-i);
        /*
        La funzione fscanf, a differenza di scanf, ritorna un valore intero
        che rappresenta il numero di caratteri letti. 
        In caso di insuccesso la funzione restituisce una costante intera EOF
        (end of life).
        */
        if (fscanf(stdin, "%d", &val) != 1) {
            perror("fscanf");
            return EXIT_FAILURE;
        }

        fprintf(stdout, "Somma: %d\n", somma(val));
        fprintf(stdout, "Somma rientrante: %d\n", somma_r(val, &s));
    }

    return 0;
}