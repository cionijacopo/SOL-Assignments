/*
Created by Jacopo Cioni on 30/3/2023
Esercizio 3 - Assignment 2
*/

#define _POSIX_C_SOURCE  200112L
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define MY_ARGV_MAX 512

// Il prototipo e' necessario perche' le funzioni che seguono non rientrano
// nello standard ISO c99 c11 
// in alternativa avrei potuto compilare definedo: -D_POSIX_C_SOURCE=200112L
char *strndup(const char *, size_t);
char *strtok_r(char *str, const char *delim, char **saveptr);

int main (int argc, char *argv[]) {
    if (argc != 3) {
        printf("Utilizzare: %s string1 string2\n", argv[0]);
        return -1;
    }
    // Faccio un controllo sulle stringhe in input, non devono essere
    // troppo lunghe
    if (strlen(argv[1]) > MY_ARGV_MAX || strlen(argv[2]) > MY_ARGV_MAX) {
        printf("Le stringhe sono troppo lunghe.\n");
        // Terminazione programma
        return -1;
    }

    // Mi copio la argv[1] perchè la strktok_r modifica la stringa
    // in ingresso
    char *str1 = strndup(argv[1], MY_ARGV_MAX);

    char *stato1 = NULL; //Stato della prima strtok_r
    char *stato2 = NULL; //Stato della seconda strtok_r
    char *token1 = strtok_r(str1, " ", &stato1);
    while (token1) {
        printf("%s\n", token1);

        char *str2 = strndup(argv[2], MY_ARGV_MAX);
        char *token2 = strtok_r(str2, " ", &stato2);

        while (token2) {
            printf("%s\n", token2);
            token2 = strtok_r(NULL, " ", &stato2);
        }
        free(str2);
        token1 = strtok_r(NULL, " ", &stato1);
    }
    free(str1);
    return 0;
}