#if !defined(CONNECTION_H)
#define CONNECTION_H

#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define SOCKNAME     "./cs_sock"
#define MAXBACKLOG   32

/** Evita letture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la lettura da fd leggo EOF
 *   \retval size se termina con successo
 */
// Da quello che ho capito nei .h vanno sempre static inline le funzioni
// Passo il file descriptor, il buffer e la dimensione
static inline int readn(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*) buf;
    while (left > 0) {
        // Read ritorna il numero di elementi letti.
        if ((r = read((int)fd, bufptr, left)) == -1) {
            if (errno = EINTR) continue;
            return -1;
        }
        if (r == 0) {
            // Sono alla fine del file.
            return 0; 
        }
        // tolgo i byte appena letti
        left = left -r;
        // scorro il buffer tanto quanti byte ho appena letto
        bufptr = bufptr + r;    
    }
    return size;
}

/** Evita scritture parziali
 *
 *   \retval -1   errore (errno settato)
 *   \retval  0   se durante la scrittura la write ritorna 0
 *   \retval  1   se la scrittura termina con successo
 */
// Passo il file descriptor, il buffer e la dimensione
static inline int writen(long fd, void *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*) buf;
    while(left > 0) {
        if((r = write((int)fd, bufptr, left)) == -1) {
            if (errno == EINTR) continue;
            return -1;
        }
        if(r == 0) {
            return 0;
        }
        left = left -r;
        bufptr = bufptr - r;
    }
    return -1;
}

#endif