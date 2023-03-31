/*
Created by Jacopo Cioni on 31/3/2023
Esercizio 1 - Assignment 3
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<tokenizer.h>

int main (int argc, char *argv[]) {
    int i;
    for(i=1;i<argc;i++) {
        char *str = strndup(argv[i], strlen(argv[i]));
        tokenizer(str, stdout);
        free(str);
    }
    for(i=1;i<argc;i++) {
        tokenizer_r(argv[i], stdout);
    }

    return 0;
} 