#include<stdarg.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>

#include"../../includes/utils.h"

// Firma
int fileno(FILE*);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Utilizzare: %s infile outfile [bufsize]\n", argv[0]);
        return EXIT_FAILURE;
    }
    char *infile = argv[1];
    char *outfile = argv[2];
    size_t bufsize = BUFSIZE;
    if (argc >= 4) {
        // atol converte stringa a long 
        bufsize = atol(argv[3]);
    }

    FILE *fin;
    // Faccio l'apertura e controllo che sia
    // andata a buon fine 
    if((fin = fopen(infile, "r")) == NULL) {
        perror("ERROR: open");
        int err = errno;
        print_error("Aprendo il file in input %s: errno=%d\n", infile, err);
        return err;
    }
    
    // Stesso giochino di impostazione della maschera, devo capire cosa fa
    mode_t oldmask = umask(033);
    FILE *fout;
    // w+ indica che voglio scrivere e nel caso creare se non c'è, cancella
    // ogni volta che apre
    if ((fout = fopen(outfile, "w+")) == NULL) {
        perror("ERROR: open");
        int err = errno;
        print_error("Creando il file in output %s: errno=%d\n", infile, errno);
        return err;
    }
    // Ripristino la maschera
    umask(oldmask);
    char *buf = malloc(bufsize * sizeof(char));
    if (!buf) {
        perror("malloc");
        return errno;
    }
    size_t len = 0;
    while ((len = fread(buf, 1, bufsize, fin)) > 0) {
        if(fwrite(buf, 1, len, fout) != len) {
            perror("ERROR: write");
            int err = errno;
            print_error("Scriviamo il file in output %s: errno=%d\n", outfile, errno);
            return err;
        }
    }
    fclose(fin);
    fflush(fout);
    fclose(fout);
    free(buf);

    return 0;
}