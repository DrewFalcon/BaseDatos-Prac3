#include "deleted.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

/* -------------------------------------------------------
 *  Funciones de comparacion para qsort
 * ------------------------------------------------------- */

static int cmp_bestfit(const void* a, const void* b) {
        const indexdeletedbook* x = a;
        const indexdeletedbook* y = b;
        return (int)x->size - (int)y->size;
}

static int cmp_worstfit(const void* a, const void* b) {
        const indexdeletedbook* x = a;
        const indexdeletedbook* y = b;
        return (int)y->size - (int)x->size;
}

DeletedList* deleted_create(int strategy) {
        DeletedList* lst = malloc(sizeof(DeletedList));
        if (!lst) return NULL;

        lst->strategy = strategy;
        lst->count = 0;
        lst->capacity = 10;
        lst->array = malloc(sizeof(indexdeletedbook) * lst->capacity);

        return lst;
}

void deleted_free(DeletedList* lst) {
        if (!lst) return;
        free(lst->array);
        free(lst);
}

void deleted_insert(DeletedList* lst, size_t size, long int offset) {
        if (lst->count == lst->capacity) {
                lst->capacity *= 2;
                lst->array = realloc(lst->array, sizeof(indexdeletedbook) * lst->capacity);
        }

        lst->array[lst->count].size = size;
        lst->array[lst->count].offset = offset;
        lst->count++;

        // First-fit: no ordenar
        if (lst->strategy == BESTFIT)
                qsort(lst->array, lst->count, sizeof(indexdeletedbook), cmp_bestfit);
        else if (lst->strategy == WORSTFIT)
                qsort(lst->array, lst->count, sizeof(indexdeletedbook), cmp_worstfit);
}

Status sort_deleted(DeletedList* lst) {
        if (!lst) return ERROR;

        if (lst->strategy == BESTFIT)
                qsort(lst->array, lst->count, sizeof(indexdeletedbook), cmp_bestfit);

        else if (lst->strategy == WORSTFIT)
                qsort(lst->array, lst->count, sizeof(indexdeletedbook), cmp_worstfit);

        return OK;
}

long int deleted_find_fit(DeletedList* lst, size_t size) {
        long int offset = 0;
        if (!lst || lst->count == 0) return -1;

        // FIRST-FIT no ordena
        if (lst->strategy == BESTFIT || lst->strategy == WORSTFIT) {
                sort_deleted(lst);
        }

        for (int i = 0; i < lst->count; i++) {
                if (lst->array[i].size < size) continue;

                offset = lst->array[i].offset;

                /* ---- HUECO EXACTO ---- */
                if (lst->array[i].size == size) {
                        /* borrar entrada desplazando elementos*/
                        for (int j = i; j < lst->count - 1; j++) {
                                lst->array[j] = lst->array[j + 1];
                        }

                        lst->count--;
                        return offset;
                }

                /* ---- HUECO MAYOR ---- */
                lst->array[i].offset += size;
                lst->array[i].size -= size;

                return offset;
        }

        return -1;
}

void deleted_load(DeletedList* lst, const char* filename) {
        FILE* f = fopen(filename, "rb");
        if (!f) return;

        fread(&lst->strategy, sizeof(int), 1, f);
        fread(&lst->count, sizeof(int), 1, f);

        lst->capacity = lst->count > 0 ? lst->count : 10;
        lst->array = realloc(lst->array, sizeof(indexdeletedbook) * lst->capacity);

        fread(lst->array, sizeof(indexdeletedbook), lst->count, f);

        fclose(f);
}

void deleted_save(DeletedList* lst, const char* filename) {
        FILE* f = fopen(filename, "wb");
        if (!f) return;

        fwrite(&lst->strategy, sizeof(int), 1, f);
        fwrite(&lst->count, sizeof(int), 1, f);
        fwrite(lst->array, sizeof(indexdeletedbook), lst->count, f);

        fclose(f);
}
