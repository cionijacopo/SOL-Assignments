//
// Created by Jacopo Cioni on 24/03/23.
// Esercizio 3 - Assignment 1
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// Header che contiene la variabile errno che viene modificata in automatico da systemCalls o da funzioni di libreria
// quando si presenta un errore
#include <errno.h>

int isNumber(const char *s, long *n) {
    // Controllo che il puntatore punti ad una porzione di memoria
    if(s==NULL) return 1;
    // Controllo che la stringa non sia vuota
    if(strlen(s) == 0) return 1;
    char *e = NULL;
    // Dichiaro la presenza della variabile di errore di errno.h
    errno=0;
    long val = strtol(s, &e, 10);
    // In caso di overflow ritorno 2.
    if (errno == ERANGE) return 2;
    if (e != NULL && *e == (char)0) {
        *n = val;
        // Terminazione con successo
        return 0;
    }
    // Terminazione fallita, non è un numero.
    return 1;
}

void print_usage(const char *program_name) {
    printf("Utilizzo: %s -n <num> -m <num> -s <string> -h\n", program_name);
}

int main(int argc, char *argv[]) {
    // Il primo elemento del vettore di argomenti è sempre un puntatore al programma corrente.
    char *program_name = argv[0];
    // Controllo che ci sia il numero giusto di argomenti.
    if(argc == 1) {
        printf("Non ci sono argomenti passati al programma. Uscita.\n");
        return 0;
    }

    char c;
    // Variabili per tenere la quantità di numeri trovati
    char found_n = 0, found_m = 0, found_s = 0;
    long narg = -1, marg = -1;
    char *sarg = NULL;
    while (--argc > 0) {
        if ((*++argv)[0] == '-') {
            while ((c = *++argv[0]) == '-');
            switch(c) {
                case 'n': {
                    found_n = 1;
                    if (argv[0][1] == '\0') {
                        ++argv, --argc;
                        if (argv[0] == NULL || (isNumber(argv[0], &narg)) != 0) {
                            found_n = 0;
                            printf("Argomento n non valido.\n");
                        }
                    } else {
                        if (isNumber(&argv[0][1], &narg) != 0) {
                            found_n = 0;
                            printf("Argomento n non valido.\n");
                        }
                    }
                } break;
                case 'm': {
                    found_m = 1;
                    if (argv[0][1] == '\0') {
                        ++argv, --argc;
                        if (argv[0] == NULL || (isNumber(argv[0], &marg)) != 0) {
                            found_m = 0;
                            printf("Argomento m non valido.\n");
                        }
                    } else {
                        if (isNumber(&argv[0][1], &marg) != 0) {
                            found_m = 0;
                            printf("Argomento m non valido.\n");
                        }
                    }
                } break;
                case 's': {
                    found_s = 1;
                    if (argv[0][1] == '\0') {
                        ++argv, --argc;
                        // strdup effettua una copia della stringa passata come parametro allocando spazio in memoria
                        if (argv[0]) sarg = strdup(argv[0]);
                    } else {
                        sarg = strdup(&argv[0][1]);
                    }
                    if (sarg == NULL) {
                        found_s = 0;
                        printf("Argomento s non valido.\n");
                    }
                } break;
                case 'h': {
                    print_usage(program_name);
                    return 0;
                } break;
                default:
                    printf("Argomento %c non riconosciuto\n", c);
            }
        }
    }
    if (found_n) {
        printf("-n: %ld\n", narg);
    }
    if (found_m) {
        printf("-m: %ld\n", marg);
    }
    if (found_s) {
        printf("-s: %s\n", sarg);
    }
    free(sarg);

    return 0;
}

