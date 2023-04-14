/*
Created by Jacopo Cioni on 14/4/2023
Esercizio 1 - Assignment 6
*/

#define _POSIX_C_SOURCE 200112L
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/wait.h>
#include<sys/types.h>

// Massima lunghezza della stringa passata come comando
#define MAX_INPUT_LINE 1024
// Numero massimo di parametri consentiti per il comando
#define MAX_PARAMETRO 256

// Macro per l'allocazione della malloc e rispettivo msg di errore
#define MALLOC(ptr, sz)                     \
    if((ptr = (char*)malloc(sz)) == NULL) { \
        perror("malloc");                   \
        exit(EXIT_FAILURE);                 \
    }

void print_prompt() {
    printf("> ");
    // Faccio il flush per ripulire
    fflush(stdout);
}

// Il primo parametro è un puntatore al buffer
// Il secondo parametro è un puntatore agli argomenti
// RICORDA: Tutto questo spazio allocato dovrà poi essere liberato.
void build_args(char *line, char *argv[]) {
    int i = 0;
    char *tmpstr, *token;
    // Richiamo la string tokenizer per separare tutti gli argomenti 
    // che hanno lo spazio
    // Salvo tutto sulla tmpstr
    // RICORDA: la prima volta che chiamo strtok_r il primo parametro deve
    // essere presente, le volte successive deve essere NULL perchè
    // riprende dallo stato precedente.
    token = strtok_r(line, " ", &tmpstr);
    size_t len = strlen(token)+1;
    // Alloco lo spazio necessario per ogni argomento utilizzando i
    MALLOC(argv[i], len); 
    // Copio il token per intero dentro lo spazio appena allocato
    strncpy(argv[i], token, len);
    // Il primo parametro di strtok_r è NULL come detto prima
    while ((token = strtok_r(NULL, " ", &tmpstr)) != NULL) {
        i++;
        // Mi preparo ad allocare lo spazio per l'argomento successivo
        len = strlen(token) + 1;
        MALLOC(argv[i], len);
        // Copio il token per intero dentro lo spazio appena allocato
        strncpy(argv[i], token, len);
    }
    argv[i+1] = NULL;
}

void clean_args(char *argv[]) {
    int i = 0;
    while (argv[i] != NULL) {
        free(argv[i]);
        i++;
    }
}

int main() {
    int pid;
    int childforked = 0;
    int status;
    char inputline[MAX_INPUT_LINE];
    char *argv[MAX_PARAMETRO];

    //CICLO INFINITO PER ESECUZIONE CONTINUA DELLA SHELL
    while(1) {
        print_prompt();
        /*
            Procedura della string.h
            Effettua una copia del parametro nel mezzo sui primi n parametri
            del primo parametro passato alla procedura. In questo caso copia lo
            0 su tutti le celle di inputline.
        */
        memset(inputline, 0, MAX_INPUT_LINE);
        // STDIN_FILENO è il file descriptor dello standard input
        /*
        La read legge dal primo parametro (STDIN_FILENO, quindi dallo standard input)
        ultimo parametro caratteri (MAX_INPUT_LINE) e li memorizza nel secondo
        parametro (buffer, in questo caso "inputline"). Ritorna il numero di 
        elementi letti, -1 in caso di errore.
        */
        if (read(STDIN_FILENO, inputline, MAX_INPUT_LINE) == -1) {
            perror("Leggendo dallo standard input: ");
            continue;
        }
        // Setto l'ultimo carattere come carattere di fine stringa
        inputline[strlen(inputline)-1] = '\0';
        if(inputline[0] != '\0') {
            build_args(inputline, argv);
        } else {
            continue;
        }

        /*
        La strncmp confronta i primi n caratteri e restituisce 0 se tutto
        va bene.
        */
        if(strncmp(inputline, "exit", 4) == 0) {
            clean_args(argv);
            break;
        }
        // Se la fork fallisce ritorna -1, per questo faccio il controllo.
        if((pid = fork()) < 0) {
            perror("fork");
            continue;
        }
        // Teoricamente entro qui dentro se sono il figlio
        if (pid == 0) {
            execvp(argv[0], &argv[0]);
            // Le istruzioni dopo vengono eseguite solo se execvp fallisce
            perror("execvp");
            exit(errno);
        }
        // Contatore dei figli
        childforked++;

        //Gestione della terminazione, se ritorna -1 c'è errore e setta errno
        // Qui sono il processo padre
        /*
         -> Con -1 come primo parametro gli sto dicendo che attendo un qualsiasi
           processo figlio
         -> Status prende il codice di ritorno sullo stato ed altre info
         -> 0 indica che non ci sono flags. Il padre sta in attesa fino a che
           il figlio non ha terminato
        */
        if ((pid = waitpid(-1, &status, 0)) != -1) {
            if (pid != 0) {
                // è il pid del processo figlio atteso
                // riduco il contatore
                childforked--;
                // Questo sotto restituisce 1 se il processo figlio
                // è terminato con successo.
                if (WIFEXITED(status)) {
                    fprintf(stdout, "Processo %d terminato con ", pid);
                    // Questo sotto mi restituisce lo stato 
                    fprintf(stdout, "exit(%d)\n", WEXITSTATUS(status));
                    // Adesso devo fare il flush dello stdout prima di chiamare la 
                    // successiva exec
                    fflush(stdout);
                }
            }
        }
        clean_args(argv); 
    }

    /* A questo punto sono nella terminazione del programma. Sarebbe cosa
    buona e giusta accertarmi di aver ricevuto tutti i processi figli che
    avevo generato con la fork. Nel caso in cui non avessero finito vanno
    terminati comunque senza starli ad aspettare. Per fare questo 
    utilizzerò il flag WNOHANG.
    */
   while (childforked > 0) {
    if ((pid = waitpid(-1, &status, WNOHANG)) != -1) {
        if(pid != 0) {
            childforked--;
            if(WIFEXITED(status)) {
                fprintf(stdout, "Processo %d terminato con ", pid);
                // Questo sotto mi restituisce lo stato 
                fprintf(stdout, "exit(%d)\n", WEXITSTATUS(status));
                // Adesso devo fare il flush dello stdout prima di chiamare la 
                // successiva exec
                fflush(stdout);
            }
        }
    }
   }
   printf("shell terminata .... \n");
   return 0;
}