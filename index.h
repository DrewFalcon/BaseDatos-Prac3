#ifndef INDEX_H
#define INDEX_H

#include <stdio.h>
#include <stdlib.h>

/* Estructura del índice para cada libro*/
typedef struct {
    int key;            /* bookID isbn*/
    long int offset;    /* posición en el .db */
    size_t size;        /* tamaño del registro. "This is redundant" */
} indexbook;

/* Estructura que maneja el índice en memoria */
typedef struct {
    indexbook *array;   /* array dinámico */
    int count;          /* número actual de entradas */
    int capacity;       /* capacidad del array */
} Index;

// Funciones mínimas del índice
Index* index_create();
void index_free(Index *idx);
void index_insert(Index *idx, int key, long int offset, size_t size);
int index_find(Index *idx, int key);
void index_load(Index *idx, const char *filename);
void index_save(Index *idx, const char *filename);

#endif