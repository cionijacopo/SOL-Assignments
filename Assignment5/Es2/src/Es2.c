#include <stdio.h>
#include<sys/types.h>
#include<dirent.h>
#include<sys/stat.h>
#include<unistd.h>
#include<string.h>
#include<time.h>

#include"../../includes/utils.h"

#if !defined(NAME_MAX)
#define NAME_MAX 256
#endif

static int quiet = 0;

int isdot (const char dir[]) {
    int l = strlen(dir);
    // Guardo se l'ultimo carattere è un punto
    if((l>0 && dir[l-1] == '.')) return 1;
    return 0;
}

char* cwd() {
  char* buf = malloc(NAME_MAX*sizeof(char));
  if (!buf) {
    perror("cwd malloc");
    return NULL;
  }
  if (getcwd(buf, NAME_MAX) == NULL) {
    if (errno==ERANGE) { // il buffer e' troppo piccolo, lo allargo
      char* buf2 = realloc(buf, 2*NAME_MAX*sizeof(char));
      if (!buf2) {
	perror("cwd realloc");
	free(buf);
	return NULL;
      }
      buf = buf2;
      if (getcwd(buf,2*NAME_MAX)==NULL) { // mi arrendo....
	if (!quiet) perror("cwd eseguendo getcwd");
	free(buf);
	return NULL;
      }
    } else {
      if (!quiet) perror("cwd eseguendo getcwd");
      free(buf);
      return NULL;
    }
  }
  return buf;
}

/*
Ritorna 0 se non è riuscito ad entrare nella directory
Ritorna 1 se va tutto bene
Ritorna -1 se c'è un errore
*/
int find(const char nomedir[], const char nomefile[]) {
    // Entro nella directory
    // chdir cambia la directory entrando in quella specificata
    // nel pathname
    if (chdir(nomedir) == -1) {
        if(!quiet) {
            print_error("Impossibile entrare nella directory %s\n", nomedir);
            return -1;
        }
    }
    DIR *dir;
    // Opendir apre la directory specificata, nel caso del punto
    // apro la directory corrente. Restituisce un puntatore allo 
    // stream della directory
    if((dir = opendir(".")) == NULL) {
        if (!quiet) {
            print_error("Errore aprendo la directory %s\n", nomedir);
            return -1;
        } }else {
            // Rappresenta lo stream della directory
            struct dirent *file;
            while ((errno = 0, file = readdir(dir)) != NULL) {
                struct stat statbuf;
                if (stat(file->d_name, &statbuf) == -1) {
                    if (!quiet) {
                        perror("stat");
                        print_error("Errore facendo stat di %s\n", file->d_name);
                    }
                    return -1;
                }
                if(S_ISDIR(statbuf.st_mode)) {
                    if(!isdot(file->d_name)) {
                        if (find(file->d_name, nomefile) != 0) {
                            if (chdir("..") == -1) {
                                print_error("Impossibile risalire alla directory padre.\n");
                                return -1;
                            }
                        }
                    }
                } else {
                    if (strncmp(file->d_name, nomefile, strlen(nomefile)) == 0) {
                        char *buf = cwd();
                        if (buf == NULL) return -1;
                        printf("%s/%s %s", buf, file->d_name, ctime(&statbuf.st_mtime));
                        free(buf);
                    }
                }
            }
            if (errno != 0) perror("readdir");
            closedir(dir);
        }
        return 1;
}

int main (int argc, char *argv[]) {
    if(argc < 3 || argc > 4) {
        fprintf(stderr, "Utilizzare: %s dir filename [quiet]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *dir = argv[1];
    const char *file = argv[2];
    if(argc == 4) quiet = 1;

    //Controllo i due argomenti
    struct stat statbuf;
    int r;
    // stat restituisce le informazioni sul file specificato
    // in questo caso dir e le salva su statbuf, su r viene memorizzato -1 se va male 
    SYSCALL_EXIT(stat, r, stat(dir, &statbuf), "Facendo stat del nome %s: errno=%d\n", dir, errno);
    if (!S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "%s non è una directory\n", dir);
        return EXIT_FAILURE;
    }
    find(dir, file);
    return 0;
}

