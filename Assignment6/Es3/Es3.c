/*
Created by Jacopo Cioni on 14/4/2023
Esercizio 3 - Assignment 6
*/

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main (int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Utilizzare: %s <N>\n", argv[0]);
        return -1;
    }

    for(int i = 0; i<atoi(argv[1]); i++) {
        if (fork() == 0) exit(0);
    }

    printf("Terminazione del programma in 50s\n");
    sleep(50);
    return 0;
}