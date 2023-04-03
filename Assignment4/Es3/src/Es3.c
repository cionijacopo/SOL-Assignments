/*
Created by Jacopo Cioni on 3/4/2023
Esercizio 3 - Assignment 4
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
// Contiene la funzione isspace()
#include<ctype.h>
#include<getopt.h>
#include "../includes/utils.h"

// Lunghezza massima per una singola lettura di linea 
#define DEFAULT_MAX_LINE 2048

int wcount (const char buf[]) {
    // Serve per preservare lo stato tra due invocazioni
    static int flag = 0;
    char *p = (char*)&buf[0];
    int cont = 0;

    // avevo iniziato a contare una parola nella chiamata precedente
    // pero' ero arrivato alla fine del buffer senza incontrare uno spazio
    // oppure '\n'
    if (flag && isspace(*p)) {
        flag = 0;
        cont = 1;
    }
    // togliamo tutti gli spazi ed i caratteri di controllo
    // iniziali in modo da iniziare con un carattere
    while(*p != '\0' && (iscntrl(*p) || isspace(*p))) {
        p++;
    }
    // Scorro la stringa
    while(*p != '\0') {
        if (isspace(*p)) {
            if (!isspace(*(p-1))) {
                cont ++;
                flag = 0;
            }
        } else {
            flag = 1;
        }
        p++;
    }
    return cont;
}

int main (int argc, char *argv[]) {
    if(argc == 1) {
        fprintf(stderr, "Utilizzare: %s [-l -w -m <num>] <nome_file> [<nome_file> ...]\n", argv[0]);
        fprintf(stderr, "-l conta il numero di linee\n");
        fprintf(stderr, "-w conta il numero di parole\n");
        fprintf(stderr, "-m <num> setta la lunghezza massima di una linea a <num>. Il valore di default è: %d\n", DEFAULT_MAX_LINE);
        exit(EXIT_FAILURE);
    }
    int cont_linea = 0;
    int cont_parola = 0;
    long lunghezza_max = DEFAULT_MAX_LINE;
    char opt;

    // Codice per il parsing della linea di comando.
    // I due punti indicano che c'è un argomento dopo la lettera
    // I due punti iniziali mi permettono di finire in : invece che ? in
    // caso di argomento mancante.
    while ((opt = getopt(argc, argv, ":lwm:")) != -1) {
        switch (opt) {
        case 'l': {
            cont_linea = 1;
        } break;
        case 'w': {
            cont_parola = 1;
        } break;
        case 'm': {
            if (isNumber(optarg, &lunghezza_max) != 0) {
                fprintf(stderr, "ATTENZIONE: l'argomento di -m non è valido.\n");
            }
            if (lunghezza_max <= 1) {
                fprintf(stderr, "ERRORE: il buffer deve essere lungo almeno 2.\n");
                exit(EXIT_FAILURE);
            }
        } break;
        case ':': {
            fprintf(stderr, "L'opzione -m necessita di un argomento numerico.\n");
        } break;
        case '?': {
            fprintf(stderr, "L'opzione '-%c' non è riconosciuta.", optopt);
        } break; 
        default: ;
        }
    }

    // Se non ci sono opzioni considero sia -l che -w abilitati
    if (cont_linea + cont_parola == 0) {
        cont_linea = 1;
        cont_parola = 1;
    }
    // A questo punto mi occupo di allocare il buffer per la lettura della parola
    char *linea = malloc(lunghezza_max*sizeof(char));
    if (!linea) {
        perror("Malloc");
        return EXIT_FAILURE;
    }
    // Eseguo la lettura per ogni file.
    // optind è l'indice del primo elemento in argv a non essere 
    // una option.
    while (argv[optind] != NULL) {
        FILE *fp;
        long num_linee, num_parole;
        size_t len;
        char *file_name = argv[optind];
        if ((fp = fopen(file_name, "r")) == NULL) {
            perror("Fopen");
            return EXIT_FAILURE;
        }
        num_linee = 0;
        num_parole = 0;
        while (fgets(linea, lunghezza_max*sizeof(char), fp) != NULL) {
            if ((len = strlen(linea)) && (linea[len-1] == '\n')) {
                num_linee++;
            }
            if (cont_parola) {
                num_parole = num_parole + wcount(linea);
            }
        }
        fclose(fp);
        if (cont_linea && !cont_parola) {
            printf("%5ld %s\n", num_linee, argv[optind]);
        }
        if (!cont_linea && cont_parola) {
            printf("%5ld %s\n", num_parole, argv[optind]);
        }
        if (cont_linea && cont_parola) {
            printf("%5ld %5ld %s\n", num_linee, num_parole, argv[optind]);
        }
        optind++;
    }
    if(linea) {
        free(linea);
    }

    return EXIT_SUCCESS;
}