//
// Created by Jacopo Cioni on 24/03/23.
// Esercizio 2.1 - Assignment 1
// RIALLOCA è una procedura
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

const int REALLOC_INC = 16;

void rialloca (char **buffer, size_t newsize) {
    // Rialloco il buffer con la nuova dimensione.
    /*
     * INFO REALLOC:
     * @params: puntatore alla porzione di memoria da riallocare
     * @params: nuova dimensione della porzione di memoria
     * @effects: rialloca lo spazio in memoria in base alla dimensione fornita
     */
    *buffer = realloc(*buffer, newsize);
    // Se il buffer non punta a niente l'allocazione non ha avuto successo.
    if (!*buffer) {
        printf("La realloc non ha avuto successo: MEMORIA ESAURITA. Uscita. \n");
        // Uscita forzata
        exit(-1);
    }
}

void *mystrcat(char *buffer, size_t size, char *first, ...){
    va_list list;
    va_start(list, first);
    if (size < strlen(first)+1) {
        rialloca(&buffer, size + strlen(first)+1+REALLOC_INC);
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
            rialloca(&buffer, strlen(buffer) + strlen(s)+1 + REALLOC_INC);
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
    rialloca(&buffer, REALLOC_INC);
    // Il buffer inizialmente è vuoto, aggiungo il carattere di fine stringa.
    // In questo modo mi assicuro anche che il buffer contenga una stringa.
    buffer[0] = '\0';
    buffer = mystrcat(buffer, REALLOC_INC, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL);

    printf("Stringa concatenata: %s\n", buffer);
    free(buffer);

    return 0;
}