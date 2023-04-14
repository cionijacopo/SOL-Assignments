/*
Created by Jacopo Cioni on 14/4/2023
Esercizio 4 - Assignment 6
*/
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/wait.h>

void inizia_termina (int n) {
    int ppid = getpid();
    // Controllo se ci sono discendenti, senno ho finito direttamente.
    if (n > 0) {
        for(int i = 0; i<n; i++) {
            // Stampa delle linette
            fprintf(stdout, "-");
        }
        fprintf(stdout, " %d: Creo un processo figlio\n", ppid);
        // Sistemo lo standard output
        fflush(stdout);
        int pid;
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Se entro qui dentro sono il figlio
            // Chiamata ricorsiva, un pò difficile come concetto.
            inizia_termina(n-1);
        } else {
            // Se entro qui dentro non sono il figlio ma sono il padre
            // (processo attuale)
            int status;
            // Aspetto il processo pid e memorizzo lo status. Non ci sono flags
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                return;
            }
            // WIFEXITED restituisce 1 se il figlio è terminato con successo
            if (!WIFEXITED(status)) {
                fprintf(stdout, "%d: figlio %d terminato con FALLIMENTO (%d)\n", ppid, pid, WEXITSTATUS(status));
                fflush(stdout);
            }
            for(int i = 0; i<(n-1); i++) {
                fprintf(stdout, "-");
            }
            if (n>1) {
                fprintf(stdout, " ");
            }
            fprintf(stdout, "%d: Terminato con successo (%d)\n", pid, WEXITSTATUS(status));
            fflush(stdout);
        }
    } else {
        fprintf(stdout, "%d: Sono l'ultimo discendente\n", getpid());
        // Non è necessario fare il flush
    }
}

int main (int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Utilizzare: %s <N>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int n = atoi(argv[1]);
    // Prendo il pid del processo padre di tutti, ovvero il main
    int pid = getpid();
    // Esegui il programma di generazione dei figli e di stampa.
    inizia_termina(n);
    if (pid == getpid()) {
        for(int i = 0; i< n; i++) {
            fprintf(stdout, "-");
        }
        fprintf(stdout, " %d: Terminato con successo (0)\n", pid);
        fflush(stdout);
    }
    return 0;
}