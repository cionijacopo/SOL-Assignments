// Definisco l'header
#if !defined(_UTILS_H)
#define _UTILS_H

#include<stdlib.h>
#include<stdio.h>

#define CHECK_EQ_EXIT(X, val, str)  \
    if((X) == val) {                \
        perror(#str);               \
        exit(EXIT_FAILURE);         \
    }

#define CHECK_NEQ_EXIT(X, val, str) \
    if((X) != val) {                \
        perror(#str);               \
        exit(EXIT_FAILURE);         \
    }

int isNumber(const char* s, long* n);

#endif