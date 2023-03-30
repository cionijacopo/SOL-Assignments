/*
Created by Jacopo Cioni on 30/3/2023
Esempio di utilizzo della tokenizer non rientrante
*/


#define _POSIX_C_SOURCE 200112L
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void tokenizer (char *stringa) {
    // Puntatore token al primo pezzo che trovo nella stringa
    char *token = strtok(stringa, " ");
    while (token) {
        printf("%s\n", token);
        token = strtok(NULL, " ");
    }
}

int main(int argc, char *argv[]) {
    int i;
    for(i=0;i<argc;i++) {
        tokenizer(argv[i]);
    }
    return 0;
}