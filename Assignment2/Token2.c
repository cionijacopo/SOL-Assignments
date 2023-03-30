/*
Created by Jacopo Cioni on 30/3/2023
Esempio di tokenizzazione di stringhe con strtok rientrante
*/

#define _POSIX_C_SOURCE 200112L
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void tokenizer_r (char *stringa) {
    char *stato;
    char *token = strtok_r(stringa, " ", &stato);
    while (token) {
        printf("%s\n", token);
        token = strtok_r(NULL, " ", &stato);
    }
}

int main (int argc, char *argv[]) {
    int i;
    for(i=1;i<argc;i++) {
        tokenizer_r(argv[i]);
    }
    return 0;
}