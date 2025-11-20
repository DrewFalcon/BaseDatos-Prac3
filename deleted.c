#include "deleted.h"

DeletedList* deleted_create(int strategy) {
    DeletedList *lst = malloc(sizeof(DeletedList));
    lst->strategy = strategy;
    lst->count = 0;
    lst->capacity = 10;
    lst->array = malloc(sizeof(indexdeletedbook) * lst->capacity);
    return lst;
}

void deleted_free(DeletedList *lst) {
    free(lst->array);
    free(lst);
}

void deleted_insert(DeletedList *lst, size_t size, long int offset) {
    if (lst->count == lst->capacity) {
        lst->capacity *= 2;
        lst->array = realloc(lst->array, sizeof(indexdeletedbook) * lst->capacity);
    }
    lst->array[lst->count].size = size;
    lst->array[lst->count].offset = offset;
    lst->count++;
}

// Por ahora: no buscar huecos; devuelve "no encontrado"
long int deleted_find_fit(DeletedList *lst, size_t size, size_t *found_size) {
    return -1;  // luego lo implementas
}

void deleted_load(DeletedList *lst, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return;

    fread(&lst->strategy, sizeof(int), 1, f);
    fread(&lst->count, sizeof(int), 1, f);

    lst->capacity = lst->count > 0 ? lst->count : 10;
    lst->array = realloc(lst->array, sizeof(indexdeletedbook) * lst->capacity);

    fread(lst->array, sizeof(indexdeletedbook), lst->count, f);
    fclose(f);
}

void deleted_save(DeletedList *lst, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    fwrite(&lst->strategy, sizeof(int), 1, f);
    fwrite(&lst->count, sizeof(int), 1, f);
    fwrite(lst->array, sizeof(indexdeletedbook), lst->count, f);

    fclose(f);
}