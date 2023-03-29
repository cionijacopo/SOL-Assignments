//
// Created by Jacopo Cioni on 28/03/23.
//

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<getopt.h>
#include<errno.h>

// Definisco il numero di opzioni che voglio gestire
#define NUMOPTIONS 4
// Definisco il limite sulla lunghezza della stringa passata come argomento
#define MY_ARGV_MAX 512

int isNumber (const char *s, long *n) {
    if(s==NULL) {
        // non punta a niente
        return 1;
    }
    if (strlen(s)==0) {
        // è vuota
        return 1;
    }
    char *e = NULL;
    errno = 0;
    long val = strtol(s, &e, 10);
    if (errno == ERANGE) {
        // Ho under/over flow
        return 2;
    }
    if (e!= NULL && *e == (char)0) {
        *n = val;
        // è un numero
        return 0;
    }
    // non è un numero
    return 1;
}

int help(const char *program_name) {
    printf("Utilizzare: %s -n <num> -m <num> -o <string> -h\n", program_name);
    return -1;
}

int arg_n(const char *n) {
    long temp;
    if (isNumber(n, &temp) != 0) {
        printf("L'argomento di n non è valido.\n");
        return -1;
    }
    printf("-n: %ld\n", temp);
    return 0;
}

int arg_m(const char *m) {
    long temp;
    if (isNumber(m, &temp) != 0) {
        printf("L'argomento di m non è valido.\n");
        return -1;
    }
    printf("-m: %ld\n", temp);
    return 0;
}

int arg_o (const char *o) {
    if(strlen(o) > MY_ARGV_MAX) {
        printf("La stringa di o è troppo lunga.\n");
        return -1;
    }
    printf("-o: \"%s\"\n", o);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Ci sono troppi pochi argomenti.\n");
        help(argv[0]);
        // Non è terminato con successo
        return -1;
    }

    int opt;
    /*
    @param: argc, numero di argomenti
    @param: argv, argomenti
    @param: *optstring, lista di caratteri 
    @effects: parsing della linea di comando.
    Utilizziamo la funzione getopt() in questo modo: 
    se il primo carattere è ":" allora vuol dire che non
    ci sono argomenti e ritorna ":"; se ritorna "?" allora
    vuol dire che c'è un'opzione non riconosciuta; se ritorna -1
    allora non ci sono altre opzioni da processare. In tutti gli alrtri
    casi ritorna il comando parsato.
    */

   //Scorro tutti gli argomenti
   while ((opt = getopt(argc, argv, ":n:m:o:h")) != -1) {
        // Gestione di tutte le casistiche
        switch (opt)
        {
        case 'n': {
            arg_n(optarg);
        } break;
        case 'm': {
            arg_m(optarg);
        } break;
        case 'o': {
            arg_o(optarg);
        } break;
        case 'h': {
            help(argv[0]);
        } break;
        case ':': {
            printf("L'opzione '-%c' richiede un argomento\n", optopt);
        } break;
        case '?': {
            printf("L'opzione '-%c' non è riconosciuta\n", optopt);
        } break;
        default:;
        }
   }
   return 0;
}
