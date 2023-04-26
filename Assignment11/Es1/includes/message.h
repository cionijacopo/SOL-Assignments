/*
    Struttura di un messaggio.
*/
#if !defined(MESSAGE_H_)
#define MESSAGE_H_

typedef struct msg {
    // Lunghezza del messaggio
    int len;
    // Messaggio
    char *str;
} msg_t;

#endif