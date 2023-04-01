# ISTRUZIONI DI COMPILAZIONE

## PRIMO STEP (creo il file oggetto)
L'opzione -fPIC dice al compilatore di generare il "Position Independent Code", in modo tale da rendere il codice generato indipendente dall'indirizzo virtuale in cui verrà caricato in memoria. È necessario per costruire librerie condivise. 
<br/>

**gcc -fPIC -I ./includes/ src/LibToken.c -o objs/tokenizer.o -c**

## SECONDO STEP (creo la libreria condivisa)
Per fare questo passaggio (e specificare che la libreria è condivisa) utilizzo il "-shared". Indico prima il nome ed il path della libreria che voglio creare e poi passo il file oggetto.
<br/>

**gcc -shared -o mylibs/libutils.so objs/tokenizer.o**

## TERZO STEP (creo l'eseguibile)
Per fare questo passaggio passo gli includes, il main e la libreria condivisa.
<br/>

**gcc -I ./includes/ -Wall -g src/MainToken.c -o ./bin/shared -L ./mylibs/ -lutils**

## QUARTO STEP (esecuzione del programma)
Per eseguire il programma devo specificare il path della libreria condivisa. Saltando questo passaggio il programma non parte.
<br/>

**LD_LIBRARY_PATH="./mylibs/" ./bin/shared**
