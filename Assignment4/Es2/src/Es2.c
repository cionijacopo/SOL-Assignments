/*
Created by Jacopo Cioni on 3/3/2023
Esercizio 2 - Assignment 4
*/

#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "../includes/utils.h"

#define DIM_MAX 512
#define MAX_NUM_LEN 128

// Definisco il tipo della funzione che utilizzo per fare il confronto.
typedef int (*F_t)(const void*, const void*, size_t);

int confronta(F_t cmp, const void *s1, const void *s2, size_t n) {
    return cmp(s1,s2,n);
}

int main(int argc, char *argv[]) {
    /*
    Il terzo argomento lo aggiungo io per fare l'aggiunta del segondo 
    programma. In questo caso se la variabile è 'crea' fa il primo step
    dell'esercizio, altrimenti se è 'controlla' fa il secondo step
    dell'esercizio.
    */
    if(argc != 3) {
        fprintf(stderr, "Utilizzare: %s <dim> <crea|controlla>", argv[0]);
        return EXIT_FAILURE;
    }
    // isNumber assegna anche il valore a dim con passaggio per 
    // riferimento.
    long dim;
    if (isNumber(argv[1], &dim) != 0) {
        fprintf(stderr, "%s non è un numero valido", argv[1]);
        return EXIT_FAILURE;
    }
    if(dim > DIM_MAX) {
        fprintf(stderr, "dim %ld è troppo grande, scegliere un valore <= %d", dim, DIM_MAX);
        return EXIT_FAILURE;
    }
    
    // Controllo il terzo parametro.
    // 0 - Controlla
    // 1 - Crea
    int crea = 0;
    if (strncmp("crea", argv[2], strlen("crea")) == 0) {
        // La stringa corrisponde proprio a crea
        // In questo caso lo faccio diventare 1
        crea = 1;
    }

    // In questo caso mi occupo di creare la matrice
    if(crea) {
        float *mat = NULL;
        // Allocazione della matrice come singolo vettore
        // Utilizzo la macro definita nelle utils.h
        // In questo caso solo il primo pezzo serve ad allocare lo spazio
        // in memoria, il resto controlla che sia == NULL. Nel caso in cui
        // fosse == NULL termino con errore perchè l'allocazione non è 
        // andata a fine.
        CHECK_EQ_EXIT(mat = (float*) malloc (dim*dim*sizeof(float)), NULL, "malloc");

        // Riempio la matrice
        // Ricordandomi che è allocata come se fosse un vettore
        for(long i = 0; i< dim; i++) {
            for(long j = 0; j < dim; j++) {
                mat[i*dim + j] = (i+j)/2.0;
            }
        }

        // genero il file per poterci scrivere sopra.
        // Parto da quello binario
        // Utilizzo lo stesso sia per il binario che per il testuale.
        FILE *fp_bin_txt;
        CHECK_EQ_EXIT(fp_bin_txt = fopen("./mat_dump.dat", "wb"), NULL, "fopen");
        // Scrivo dentro il file, controllo in questo caso di aver scritto
        // dim*dim elementi tramite la macro definita in utils.h
        CHECK_NEQ_EXIT(fwrite(&mat[0], sizeof(float), dim*dim, fp_bin_txt), (dim*dim), "fwrite");
        // Chiudo il file binario
        fclose(fp_bin_txt);

        // Passo alla scrittura del file testuale
        CHECK_EQ_EXIT(fp_bin_txt = fopen("./mat_dump.txt", "w"), NULL, "fopen");
        // Passo alla scrittura dentro il file.
        for (long i = 0; i< dim; i++) {
            for (long j = 0; j<dim; j++) {
                if (fprintf(fp_bin_txt, "%f\n", mat[i*dim + j]) < 0) {
                    perror("fprintf");
                    return EXIT_FAILURE;
                }
            }
        }
        fclose(fp_bin_txt);
        if (mat) {
            free(mat);
        }
    } else {
        //In questa sezione mi occupo del controllo.
        float *mat = NULL;
        CHECK_EQ_EXIT(mat = (float*) malloc (dim*dim*sizeof(float)), NULL, "malloc");
        float *mat_due = NULL;
        CHECK_EQ_EXIT(mat_due = (float*) malloc (dim*dim*sizeof(float)), NULL, "malloc");

        FILE *fp_uno;
        CHECK_EQ_EXIT(fp_uno = fopen("./mat_dump.dat", "rb"), NULL, "fopen mat_dump.dat");
        FILE *fp_due;
        CHECK_EQ_EXIT(fp_due = fopen("./mat_dump.txt", "r"), NULL, "fopen mat_dump.txt");

        // Definisco il buffer per la lettura
        // Questo mi serve perche il file txt sarà una stringa
        //char buf[MAX_NUM_LEN];
        for(long i = 0; i < dim; i++) {
            for (long j = 0; j< dim; j++) {
                /*
                // Leggo e Copio il file 2 nel buffer
                if (fgets(buf, MAX_NUM_LEN, fp_due) == NULL) {
                    perror("fgets");
                    exit(EXIT_FAILURE);
                }
                // Aggiungo il carattere di fine stringa
                buf[strlen(buf)-1] = '\0';
                mat[i*dim + j] = strtof(buf, NULL);
                */
                fscanf(fp_due, "%f", &mat[i*dim + j]);
            }
            fclose(fp_due);
        }

        //Adesso passo alla lettura del file binario, per fare questo
        // mi basterà utilizzare la fread.
        CHECK_NEQ_EXIT(fread(&mat_due[0], sizeof(float), dim*dim, fp_uno), (dim*dim), "fread");
        fclose(fp_uno);

        //Funzione di confronto tra le due matrici
        // Utilizzo la memcmp che compara n bytes dell'area
        // di memoria di s1 e dell'area di memoria di s2. Se ritorna 0
        // allora s1 e s2 sono uguali. 
        /*
        DEBUG:

        printf("\nPRIMA MATRICE: \n");
        for(long i=0;i<dim;i++) {
            for(long j=0; j<dim; j++) {
                printf("%f ", mat[i*dim + j]);
            }
            printf("\n");
        }
        printf("\nSECONDA MATRICE: \n");
        for(long i=0;i<dim;i++) {
            for(long j=0; j<dim; j++) {
                printf("%f ", mat_due[i*dim + j]);
            }
            printf("\n");
        }
        */

        if(confronta(memcmp, mat, mat_due, dim*dim*sizeof(float)) != 0) {
            fprintf(stderr, "Le due matrici non sono uguali.\n");
            return EXIT_FAILURE;
        }
        fprintf(stdout, "Le due matrici sono uguali.\n");
        free(mat);
        free(mat_due);
    }
    return EXIT_SUCCESS;
}