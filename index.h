#ifndef INDEX_H
#define INDEX_H

#include <stdio.h>
#include <stdlib.h>

#include "record.h"
#include "utils.h"
/* Estructura del índice para cada libro*/
typedef struct {
        int key;         /* bookID*/
        long int offset; /* posición en el .db */
        size_t size;     /* tamaño del registro */
} indexbook;

/* Estructura que maneja el índice en memoria*/
typedef struct {
        indexbook* array; /* array dinámico */
        int count;        /* número actual de entradas */
        int capacity;     /* capacidad del array */
} Index;

// Funciones mínimas del índice
Index* index_create();
void index_free(Index* idx);
void index_insert(Index* idx, int key, long int offset, size_t size);
void index_load(Index* idx, const char* filename);
void index_save(Index* idx, const char* filename);
int index_find(Index* idx, int key);
void index_print(Index* idx, FILE* db, int key, int pos);
Status index_delete(Index* idx, int key);

#endif
