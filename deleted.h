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
        indexdeletedbook* array;
        int count;
        int capacity;
} DeletedList;

// Funciones mínimas
DeletedList* deleted_create(int strategy);
void deleted_free(DeletedList* lst);
void deleted_insert(DeletedList* lst, size_t size, long int offset);
long int deleted_find_fit(DeletedList* lst, size_t size);
void deleted_load(DeletedList* lst, const char* filename);
void deleted_save(DeletedList* lst, const char* filename);

#endif