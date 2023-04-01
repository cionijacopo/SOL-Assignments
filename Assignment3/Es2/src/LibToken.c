/*
Created by Jacopo Cioni on 31/3/2023
Esercizio 1 - Assignment 3

Qui si trovano le funzioni/procedure che voglio poi richiamare dal mio 
main o in generale da altre parti. In questa sezione troviamo tutte
le implementazioni.
*/

#define _POSIX_C_SOURCE 200112L
#include<stdio.h>
#include<string.h>

// Includo anche la tokenizer che poi farà da ponte
#include<tokenizer.h>

//File di Output presenti

// versione non rientrante
void tokenizer(char *stringa, FILE *out) {
    char* token = strtok(stringa, " ");
    while (token) {
	fprintf(out, "%s\n", token);
	token = strtok(NULL, " ");
    }
}

// versione rientrante
void tokenizer_r(char *stringa, FILE *out) {
    char *tmpstr;
    char *token = strtok_r(stringa, " ", &tmpstr);
    while (token) {
	fprintf(out, "%s\n", token);
	token = strtok_r(NULL, " ", &tmpstr);
    }
}