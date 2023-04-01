/*
Created by Jacopo Cioni on 1/4/2023
Esercizio 1 - Assignment 4
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

//Macro per il path
#define PASSWD_FILE "/etc/passwd"

#if !defined(MAX_LEN)
#define MAX_LEN 1024
#endif


int main (int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "too few arguments\n Utilizzare: %s out-filename\n", argv[0]);
        return -1;
    }
    FILE *fd = NULL;
    FILE *ofp = NULL;
    // Buffer per la lettura della stringa
    char *buffer = NULL;

    if((fd = fopen(PASSWD_FILE, "r")) == NULL) {
        perror("Apertura file password");
        goto error;
    }
    if((ofp = fopen(PASSWD_FILE, "w")) == NULL) {
        perror("Apertura file output");
        goto error;
    }
    if ((buffer = malloc(MAX_LEN*sizeof(char))) == NULL) {
        perror("Malloc buffer");
        goto error;
    }
    // Leggo i caratteri dal file e li copio nel buffer.
    // Continua a leggere fino al primo \n o all' EOF
    while (fgets(buffer, MAX_LEN, fd) != NULL) {
        char *newline;
        // Controllo di aver letto tutta la linea
        // Se il carattere non viene trovato strchr restituisce NULL
        // In questo caso cerco se ho il carattere di fine stringa
        // all'interno del buffer, se non c'è non ho letto tutta
        // la riga.
        if ((newline = strchr(buffer, '\n')) == NULL) {
            fprintf(stderr, "Buffer di linea troppo piccolo, aumentare MAX_LEN (%d) ricompilando.\n", MAX_LEN);
            goto error;
        }
        // Se arrivo qui l'ho letta tutta
        // Tolgo lo '\n'
        *newline = '\0';
        char *colons;
        if ((colons = strchr(buffer, ':')) == NULL) {
            fprintf(stderr, "Wrong file format.\n");
            goto error;
        }
        // Sostutuisco : con \0
        *colons = '\0';
        if (fprintf(ofp, "%s\n", buffer) != strlen(buffer)+1) {
            perror("fprintf");
            goto error;
        }
    }

    fclose(fd);
    fclose(ofp);
    free(buffer);

    return EXIT_SUCCESS;

    error: 
        if (fd) fclose(fd);
        if (ofp) fclose(ofp);
        if (buffer) free(buffer);
        exit(EXIT_FAILURE);

}