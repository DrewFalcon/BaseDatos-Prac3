#include "index.h"

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

void index_insert(Index* idx, int key, long int offset, size_t size) {
        if (idx->count >= idx->capacity) {
                idx->capacity *= 2;
                idx->array = realloc(idx->array, sizeof(indexbook) * idx->capacity);
        }
        idx->array[idx->count].key = key;
        idx->array[idx->count].offset = offset;
        idx->array[idx->count].size = size;
        idx->count++;
}

void index_load(Index* idx, const char* filename) {
        FILE* f = fopen(filename, "rb");
        if (!f) return;

        fread(&idx->count, sizeof(int), 1, f);
        idx->capacity = idx->count > 0 ? idx->count : 10;
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

int find(FILE* db, Index* idx, char* arguments) {
        /*for (int i = 0; i < idx->count; i++) {
            if (idx->array[i].key == key)
                return i;
        }*/

        int middle;
        BookRecord* rec = NULL;
        int key = atoi(arguments);

        if (idx == NULL) {
                fprintf(stdout, "Record with bookId=%i does not exist", key);
                return -1;
        }

        middle = (idx->count / 2);

        while (middle <= idx->count && middle > 0) {
                if (idx->array[middle].key == key) {
                        /*Si hemos encontrado el libro a imprimir*/
                        rec = record_read(db, idx->array[middle].offset);
                        printf("%d|%s|%s|%s\n", rec->bookID, rec->isbn, rec->title, rec->printedBy);
                        return middle;
                } else if (idx->array[middle].key < key) {
                        middle = (middle + idx->count) / 2;
                } else {
                        middle = (0 + middle) / 2;
                }
        }

        return -2;
}

Status index_delete(FILE* db, Index* idx, int key) {
        int pos = 0;
        char* string_key = NULL;
        if (!idx) {
                return ERROR;
        }

        sprintf(string_key, "%d", key);

        pos = find(db, idx, string_key);
        if (pos == -1) {
                return ERROR;
        }

        /* Mover todos los elementos a la izquierda desde pos+1 hasta el final */
        if (pos < idx->count - 1) {
                memmove(&idx->array[pos],     /* destino */
                        &idx->array[pos + 1], /* origen */
                        (idx->count - pos - 1) * sizeof(indexbook));
        }

        idx->count--; /* reducir el número de entradas */

        return OK;
}
