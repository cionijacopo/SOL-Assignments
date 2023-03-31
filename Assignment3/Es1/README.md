# ISTRUZIONI PER LA COMPILAZIONE

## PRIMO STEP (creo il file oggetto)
gcc -c -o objs/tokenizer.o src/LibToken.c -I ./includes/
## SECONDO STEP (creo la libreria statica)
ar rvs mylibs/libutils.a objs/tokenizer.o
## TERZO STEP (creo l'eseguibile)
gcc -I ./includes/ -Wall -g ./src/MainToken.c -o ./bin/static -L ./mylibs/ -lutils