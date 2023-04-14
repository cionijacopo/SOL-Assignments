#include<unistd.h>
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/wait.h>

int main(int argc, char *argv[]) {
    // Devo avere solo un parametro come argomento
    if (argc != 2) {
        fprintf(stderr, "Utilizzare: %s <sleeeptime>\n", argv[0]);
        return -1;
    }
    // Generazione del processo figlio
    int pid = fork();
    if (pid == 0) {
        // Qui sono il figlio
        if ((pid = fork()) == 0) {
            // Qui sono il figlio del figlio
            /*
                Exec prende il path e poi la lista degli argomenti da passare
                al path. La lista deve sempre terminare con NULL. Il primo 
                argomento mi è sembrato di capire che deve essere sempre uguale 
                al path (?)
            */
            execl("/bin/sleep", "/bin/sleep", argv[1], NULL);
            // Se arrivo qui c'è stato errore
            perror("execl");
            return(EXIT_FAILURE);
        }
        // Qui sono il figlio
        // Aspetto la terminazione del figlio del figlio
        // Aspetto il figlio pid
        // setto NULL lo stato perchè non mi interessa
        // Non setto nessun flag
        if (waitpid(pid, NULL, 0) < 0) {
            fprintf(stderr, "Error waiting pid.\n");
            return -1;
        }
        /*
        Il pid del padre dovrebbe essere quello del main. Il problema
        è che il main termina molto prima della scadenza del processo figlio
        e quindi questo rimane orfano e senza pid del padre (che verrà quindi
        settato automaticamente ad 1).
        */
        printf("mio = %d, padre = %d\n", getpid(), getppid());
	    return 0;
    }
    return 0;
}