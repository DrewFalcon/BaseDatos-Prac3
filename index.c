#include "index.h"

#include <stdint.h>

// Crea un índice vacío
Index* index_create() {
        Index* idx = malloc(sizeof(Index));
        idx->count = 0;
        idx->capacity = 10;
        idx->array = malloc(sizeof(indexbook) * idx->capacity);
        return idx;
}

void index_free(Index* idx) {
        free(idx->array);
        free(idx);
}

/*void index_insert(Index* idx, int key, long int offset, size_t size) {
        int pos = 0;
        if (idx->count >= idx->capacity) {
                idx->capacity *= 2;
                idx->array = realloc(idx->array, sizeof(indexbook) * idx->capacity);
        }

        while (pos < idx->count && idx->array[pos].key < key) pos++;

        if (pos < idx->count) {
                memmove(&idx->array[pos + 1], /* destino */
/*&idx->array[pos],     /* origen */
/*(idx->count - pos) * sizeof(indexbook));
}

idx->array[pos].key = key;
idx->array[pos].offset = offset;
idx->array[pos].size = size;
idx->count++;
return;
}*/

void index_insert(Index* idx, int key, long int offset, size_t size) {
        if (idx->count >= idx->capacity) {
                idx->capacity *= 2;
                idx->array = realloc(idx->array, sizeof(indexbook) * idx->capacity);
        }

        /* Buscar posición correcta con búsqueda lineal (funciona y no rompe nada). */
        int pos = 0;
        while (pos < idx->count && idx->array[pos].key < key) {
                pos++;
        }

        /* Mover los elementos UNA POSICIÓN hacia arriba */
        if (pos < idx->count) {
                memmove(&idx->array[pos + 1], &idx->array[pos], (idx->count - pos) * sizeof(indexbook));
        }

        /* Insertar */
        idx->array[pos].key = key;
        idx->array[pos].offset = offset;
        idx->array[pos].size = size;

        idx->count++;
}

void index_load(Index* idx, const char* filename) {
        FILE* f = fopen(filename, "rb");
        if (!f) return;

        fread(&idx->count, sizeof(int), 1, f);
        idx->capacity = idx->count;
        idx->array = realloc(idx->array, sizeof(indexbook) * idx->capacity);

        fread(idx->array, sizeof(indexbook), idx->count, f);

        fclose(f);
}

void index_save(Index* idx, const char* filename) {
        FILE* f = fopen(filename, "wb");
        if (!f) return;

        fwrite(&idx->count, sizeof(int), 1, f);
        fwrite(idx->array, sizeof(indexbook), idx->count, f);

        fclose(f);
}

Status index_delete(Index* idx, int key) {
        int pos = 0;

        if (!idx) {
                return ERROR;
        }

        pos = index_find(idx, key);
        if (pos == -1) {
                return ERROR;
        } /* Mover todos los elementos a la izquierda desde pos+1 hasta el final */
        if (pos < idx->count - 1) {
                memmove(&idx->array[pos], /* destino */ &idx->array[pos + 1],
                        /* origen */ (idx->count - pos - 1) * sizeof(indexbook));
        }
        idx->count--; /* reducir el número de entradas */
        return OK;
}

int index_find(Index* idx, int key) {
        int low = 0;
        int high = idx->count - 1;

        while (low <= high) {
                int mid = (low + high) / 2;

                if (idx->array[mid].key == key) return mid;

                if (idx->array[mid].key < key)
                        low = mid + 1;
                else
                        high = mid - 1;
        }

        return -1;
}

/*Este es la logica para imprimir el resultado de Find*/
void index_print(Index* idx, FILE* db, int key, int pos) {
        BookRecord* rec = NULL;

        if (idx != NULL && (idx->array) != NULL && pos >= 0 && pos < idx->count) {
                rec = record_read(db, idx->array[pos].offset);
                if (rec != NULL && pos >= 0) {
                        fprintf(stdout, "%i|%s|%s%s\n", rec->bookID, rec->isbn, rec->title, rec->printedBy);
                        free(rec);
                } else {
                        fprintf(stdout, "Record with bookId=%i does not exist\n", key);
                }
        } else {
                fprintf(stdout, "Record with bookId=%i does not exist\n", key);
        }

        return;
}