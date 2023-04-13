#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<errno.h>

#include"../../includes/utils.h"

int main(int argc, char *argv[]) {
    if(argc<3) {
        fprintf(stderr, "Utilizzare: %s infile outfile [bufsize]\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *infile = argv[1];
    char *outfile = argv[2];
    size_t bufsize = BUFSIZE;
    if(argc >= 4) {
        long size;
        if (isNumber(argv[3], &sz) != 0 || sz <= 0) {
            fprintf(stderr, "L'argomento %s non è un argomento valido\n", argv[3]);
            return EXIT_FAILURE;
        }
        bufsize = sz;
    }

    int fin;
    // Apro il file in sola lettura, tutto il resto è per gestire l'errore
    // con la macro che si trova in utils.h
    SYSCALL_EXIT(open, fin, open(infile, O_RDONLY), "Aprendo il file di input %s: errno=%d\n",infile, errno);

    // Imposto la maschera per la creazione del file.
    // Non ho ben capito l'utilizzo della maschera
    mode_t oldmask = umask(033);
    int fout;
    // Creazione del file
    SYSCALL_EXIT(open, fout, open(outfile, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH),
    "Creando il file in output %s : errno=%d\n", infile, errno);
    umask(oldmask);
    char* buf = malloc(bufsize * sizeof(char));
    if (!buf) {
        perror("malloc");
        return errno;
    }
    size_t len;
    do {
        SYSCALL_EXIT(read, len, read(fin, buf, bufsize), "Leggendo il file in input %s: errno=%d\n",infile, errno);
        if(len) {
            SYSCALL_EXIT(write, len, write(fout, buf, len), "Scrivendo il file in output %s: errno=%d\n", outfile, errno);
        }
    } while (len > 0); 
    SYSCALL_EXIT(close, len, close(fin), "Chiudendo il file in input %s: errno=%d\n", infile, errno);
    SYSCALL_EXIT(close, len, close(fout), "Chiudendo il file di output %s: errno%d\n", outfile, errno);
    free(buf);
}

