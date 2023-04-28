#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<ctype.h>
#include<assert.h>
#include<sys/select.h>

#include"../includes/connection.h"
#include"../includes/message.h"
#include"../includes/threadF.h"

// converte tutti i carattere minuscoli in maiuscoli
static void toup(char *str) {
    char *p = str;
    while(*p != '\0') { 
        *p = (islower(*p)?toupper(*p):*p); 
	++p;
    }        
}

void threadF(void *arg) {
    assert(arg);
    long *args = (long*)arg;
    long connfd = args[0];
    long *termina = (long*)(args[1]);
    free(arg);

    fd_set set, tmpset;

    //Inizializzo i set
    FD_ZERO(&set);
    FD_SET(connfd, &set);

    do{
        tmpset = set;
        int r;
        // Faccio un timeout per controllare ogni tanto la terminazione
        struct timeval timeout = (0, 100000); // 100 milliseconds
        // Faccio la select tra tutti i file descriptor accettati
        if((r = select(connfd+1, &tmpset, NULL, NULL, &timeout)) < 0) {
            perror("select");
            break;
        }
        // Se ritorna zero, allora il timeout è scaduto prima che alcun file descriptor sia pronto
        if(r== 0) {
            // Controllo se devo terminare, ovvero se termina è 1
            if(*termina) {
                break;
            }
            continue;
        }
        msg_t str;
        int n;
        if ((n = readn(connfd, &str.len, sizeof(int))) == -1) {
            perror("readn1");
            break;
        }
        if(n == 0) {
            break; // Ho finito di leggere
        }
        str.str = calloc((str.len), sizeof(char));
        if(!str.str) {
            perror("calloc");
            fprintf(stderr, "Memoria esaurita");
            break;
        }
        if((n = readn(connfd, str.str, str.len * sizeof(char))) == -1) {
            perror("readn2");
            free(str.str);
            break;
        }

        toup(str.str);

        if ((n = writen(connfd, &str.len, sizeof(int))) == -1) {
            perror("writen1");
            free(str.str);
            break;
        }
        if((n=writen(connfd, str.str, str.len*sizeof(char))) == -1) {
            perror("write2");
            free(str.str);
            break;
        }
        free(str.str);
    } while(*termina == 0);
    close(connfd);
}