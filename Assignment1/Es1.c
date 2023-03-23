//
// Created by Jacopo Cioni on 23/03/23.
// Esercizio 1 - Assignment 1
//

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
 * @params: char* in --> Puntatore alla stringa da convertire.
 * @params: size_t len --> Lunghezza della stringa, comprende il carattere vuoto.
 * @params: char* out --> Puntatore alla stringa convertita.
 * @effects: converte la stringa tramite la funzione toupper della string.h (i numeri vengono saltati)
 */
void strtoupper(const char* in, size_t len, char* out) {
    for(int i=0; i<len; i++) {
        // Memorizzo in out il risultato della funzione toupper
        out[i] = toupper(in[i]);
    }
    // Devo aggiungere il carattere finale di stringa alla fine.
    out[len] = '\0';
}

int main(int argc, char* argv[]){

    // Vengono fatti due cicli, il primo argomento (args[0]) è il riferimento al main, per questo motivo parto
    // direttamente da 1. Viene iterato una volta sola (se vengono passati più argomenti all'avvio vengono fatte
    // tante iterazioni quanti sono gli argomenti.
    for(int i=1; i<argc; i++) {
        // size_t --> è un tipo di variabile unsigned di 4byte usata per definire le grandezze di un
        // buffer, array , etc..
        size_t len = strlen(argv[i]);
        // Dichiaro un puntatore a memoria, servirà per allocare lo spazio della nuova stringa.
        char *str;
        // Alloco lo spazio per la stringa upper, il +1 è per il carattere finale.
        str = malloc(len+1);
        // Controllo che l'allocazione sia stata fatta con successo.
        if (str != NULL) {
            // Richiamo la funzione strtoupper per la conversione
            strtoupper(argv[i], len, str);
            // Stampo il prima ed il dopo
            printf("%s --> %s\n", argv[i], str);
            // Libero la memoria appena utilizzata per la conversione.
            free(str);
        } else {
            // Lancio un errore solo se non riesco ad allocare memoria
            printf("FATAL ERROR, memory exhausted\n");
            return -1;
        }
    }
    // Il return 0 mi indica che è andato tutto bene.
    return 0;
}

