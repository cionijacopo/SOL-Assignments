//
// Created by Jacopo Cioni on 24/03/23.
// Esercizio 2.2 - Assignment 1
// RIALLOCA è una macro
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define REALLOC_INC 16
/*
 * Definizione della macro. Attenzione alla sintassi.
 */
#define rialloca(buf, newsize)                                                    \
    buf = realloc(buf, newsize);                                                  \
    if(!buf) {                                                                    \
        printf("La realloc non ha avuto successo: MEMORIA ESAURITA. Uscita. \n"); \
        exit(-1);                                                                 \
    }

void *mystrcat(char *buffer, size_t size, char *first, ...){
    va_list list;
    va_start(list, first);
    if (size < strlen(first)+1) {
        rialloca(buffer, size + strlen(first)+1+REALLOC_INC);
        size = size + strlen(first)+1+REALLOC_INC;
    }
    // Concatena  i size caratteri di first alla fine del contenuto del buffer.
    strncat(buffer, first, size);
    char *s = NULL;
    // Scorro gli argomenti fino a quando non li ho visti tutti.
    while ((s = va_arg(list, char*)) != NULL) {
        // Se la dimensione necessaria è minore dell'attuale dimensione del buffer più quella della nuova stringa
        // allora devo riallocare memoria e modificare la dimensione necessaria.
        if (size < strlen(buffer) + strlen(s)+1) {
            rialloca(buffer, strlen(buffer) + strlen(s)+1 + REALLOC_INC);
            size = strlen(buffer) + strlen(s)+1 + REALLOC_INC;
        }
        // Concateno il nuovo pezzo di stringa
        strncat(buffer, s, size);
    }
    // Chiudo la lista degli elementi variabili
    va_end(list);
    return buffer;
}

int main (int argc, char* argv[]) {
    if(argc < 7) {
        printf("Ci sono troppi pochi argomenti. Riprovare.\n");
        return -1;
    }
    char *buffer = NULL;
    // La prima volta alloco una memoria definita da me come punto di partenza.
    rialloca(buffer, REALLOC_INC);
    // Il buffer inizialmente è vuoto, aggiungo il carattere di fine stringa.
    // In questo modo mi assicuro anche che il buffer contenga una stringa.
    buffer[0] = '\0';
    buffer = mystrcat(buffer, REALLOC_INC, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL);

    printf("Stringa concatenata: %s\n", buffer);
    free(buffer);

    return 0;
}
