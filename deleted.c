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
        /* BEST-FIT: orden ascendente por tamaño */
        if (x->size < y->size) return -1;
        if (x->size > y->size) return 1;
        /* si empatan, como desempate usamos el offset (no es crítico para los tests) */
        if (x->offset < y->offset) return -1;
        if (x->offset > y->offset) return 1;
        return 0;
}

static int cmp_worstfit(const void* a, const void* b) {
        const indexdeletedbook* x = a;
        const indexdeletedbook* y = b;
        /* WORST-FIT: orden descendente por tamaño */
        if (x->size > y->size) return -1;
        if (x->size < y->size) return 1;
        /* desempate por offset */
        if (x->offset < y->offset) return -1;
        if (x->offset > y->offset) return 1;
        return 0;
}

/* -------------------------------------------------------
 *  Creación / destrucción
 * ------------------------------------------------------- */

DeletedList* deleted_create(int strategy) {
        DeletedList* lst = malloc(sizeof(DeletedList));
        if (!lst) return NULL;

        lst->strategy = strategy;
        lst->count = 0;
        lst->capacity = 10;
        lst->array = malloc(sizeof(indexdeletedbook) * lst->capacity);
        if (!lst->array) {
                free(lst);
                return NULL;
        }

        return lst;
}

void deleted_free(DeletedList* lst) {
        if (!lst) return;
        free(lst->array);
        free(lst);
}

/* -------------------------------------------------------
 *  Ordenación según estrategia (BEST/WORST)
 * ------------------------------------------------------- */

Status sort_deleted(DeletedList* lst) {
        if (!lst) return ERROR;

        if (lst->strategy == BESTFIT) {
                qsort(lst->array, lst->count, sizeof(indexdeletedbook), cmp_bestfit);
        } else if (lst->strategy == WORSTFIT) {
                qsort(lst->array, lst->count, sizeof(indexdeletedbook), cmp_worstfit);
        }
        /* FIRST-FIT: NO SE ORDENA NUNCA */

        return OK;
}

/* -------------------------------------------------------
 *  Inserción de un hueco
 * ------------------------------------------------------- */

void deleted_insert(DeletedList* lst, size_t size, long int offset) {
        indexdeletedbook* tmp = NULL;

        if (!lst) return;

        /* Aseguramos capacidad */
        if (lst->count == lst->capacity) {
                lst->capacity *= 2;
                tmp = realloc(lst->array, sizeof(indexdeletedbook) * lst->capacity);
                if (!tmp) {
                        /* si realloc falla, mejor no tocar nada */
                        lst->capacity /= 2;
                        return;
                }
                lst->array = tmp;
        }

        /* Añadimos al final (inserción en orden de llegada) */
        lst->array[lst->count].size = size;
        lst->array[lst->count].offset = offset;
        lst->count++;

        /* BEST-FIT / WORST-FIT: mantenemos la lista ordenada por tamaño */
        if (lst->strategy == BESTFIT || lst->strategy == WORSTFIT) {
                sort_deleted(lst);
        }
        /* FIRST-FIT: no tocamos el orden, se queda en orden de inserción */
}

/* -------------------------------------------------------
 *  Búsqueda de hueco según estrategia
 * ------------------------------------------------------- */

long int deleted_find_fit(DeletedList* lst, size_t size) {
        long int offset = -1;
        int i = 0;

        if (!lst || lst->count == 0) return -1;

        /* Para BEST/WORST, nos aseguramos de que la lista está ordenada
           antes de buscar */
        if (lst->strategy == BESTFIT || lst->strategy == WORSTFIT) {
                sort_deleted(lst);
        }
        /* Para FIRST-FIT NO ORDENAMOS: se recorre la lista según el orden
           en el que se fueron insertando los huecos */

        for (i = 0; i < lst->count; i++) {
                /* buscamos el primer hueco cuyo tamaño sea suficiente */
                if (lst->array[i].size < size) continue;

                offset = lst->array[i].offset;

                /* ---- HUECO EXACTO ---- */
                if (lst->array[i].size == size) {
                        /* eliminamos la entrada desplazando el resto hacia la izquierda */
                        if (i < lst->count - 1) {
                                memmove(&lst->array[i], &lst->array[i + 1], (lst->count - i - 1) * sizeof(indexdeletedbook));
                        }
                        lst->count--;
                        return offset;
                }

                /* ---- HUECO MAYOR ----
                 * Reutilizamos la parte inicial y dejamos el resto como un hueco más pequeño.
                 * No creamos un nuevo hueco; simplemente ajustamos offset y size.
                 */
                lst->array[i].offset += size;
                lst->array[i].size -= size;
                return offset;
        }

        /* No se ha encontrado hueco suficientemente grande */
        return -1;
}

/* -------------------------------------------------------
 *  Carga y guardado de la lista de huecos
 * ------------------------------------------------------- */

void deleted_load(DeletedList* lst, const char* filename) {
        FILE* f;
        indexdeletedbook* tmp = NULL;

        if (!lst || !filename) return;

        f = fopen(filename, "rb");
        if (!f) return;

        /* leemos estrategia y número de entradas */
        if (fread(&lst->strategy, sizeof(int), 1, f) != 1) {
                fclose(f);
                return;
        }
        if (fread(&lst->count, sizeof(int), 1, f) != 1) {
                lst->count = 0;
                fclose(f);
                return;
        }

        /* Ajustamos capacidad y memoria */
        lst->capacity = (lst->count > 0) ? lst->count : 10;
        tmp = realloc(lst->array, sizeof(indexdeletedbook) * lst->capacity);
        if (!tmp) {
                lst->count = 0;
                fclose(f);
                return;
        }
        lst->array = tmp;

        /* Leemos todas las entradas tal cual se guardaron */
        if (lst->count > 0) {
                if (fread(lst->array, sizeof(indexdeletedbook), lst->count, f) != (size_t)lst->count) {
                        lst->count = 0;
                        fclose(f);
                        return;
                }
        }

        /* IMPORTANTE: NO ORDENAMOS aquí.
         * El fichero .lst se compara con un fichero de control mediante `diff`,
         * así que debemos respetar exactamente el orden en que se guardó.
         */

        fclose(f);
}

void deleted_save(DeletedList* lst, const char* filename) {
        FILE* f;

        if (!lst || !filename) return;

        f = fopen(filename, "wb");
        if (!f) return;

        /* Guardamos estrategia y número de entradas */
        fwrite(&lst->strategy, sizeof(int), 1, f);
        fwrite(&lst->count, sizeof(int), 1, f);

        /* Guardamos las entradas en el orden actual de memoria */
        if (lst->count > 0) {
                fwrite(lst->array, sizeof(indexdeletedbook), lst->count, f);
        }

        fclose(f);
}
