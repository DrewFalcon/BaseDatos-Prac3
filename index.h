#ifndef INDEX_H
#define INDEX_H

#include <stdio.h>
#include <stdlib.h>

/* Estructura del índice para cada libro*/
typedef struct {
    int key;            /* bookID*/
    long int offset;    /* posición en el .db */
    size_t size;        /* tamaño del registro */
} indexbook;

// Estructura que maneja el índice en memoria
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
#ifndef DELETED_H
#define DELETED_H

#include <stdio.h>
#include <stdlib.h>

// Estrategias
#define BESTFIT 0
#define WORSTFIT 1
#define FIRSTFIT 2

// Entrada de la lista de huecos
typedef struct {
    size_t size;
    long int offset;
} indexdeletedbook;

// Lista completa de huecos
typedef struct {
    int strategy;
    indexdeletedbook *array;
    int count;
    int capacity;
} DeletedList;

// Funciones mínimas
DeletedList* deleted_create(int strategy);
void deleted_free(DeletedList *lst);
void deleted_insert(DeletedList *lst, size_t size, long int offset);
long int deleted_find_fit(DeletedList *lst, size_t size, size_t *found_size);
void deleted_load(DeletedList *lst, const char *filename);
void deleted_save(DeletedList *lst, const char *filename);

#endif


