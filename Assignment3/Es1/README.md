# ISTRUZIONI PER LA COMPILAZIONE

## PRIMO STEP 
gcc -c -o objs/tokenizer.o src/LibToken.c -I ./includes/
## SECONDO STEP
ar rvs mylibs/libutils.a objs/tokenizer.o
## TERZO STEP
gcc -I ./includes/ -Wall -g ./src/MainToken.c -o ./bin/static -L ./mylibs/ -lutils