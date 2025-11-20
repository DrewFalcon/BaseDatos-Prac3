#include "index.h"

// Crea un índice vacío
Index* index_create() {
    Index *idx = malloc(sizeof(Index));
    idx->count = 0;
    idx->capacity = 10;
    idx->array = malloc(sizeof(indexbook) * idx->capacity);
    return idx;
}

void index_free(Index *idx) {
    free(idx->array);
    free(idx);
}

void index_insert(Index *idx, int key, long int offset, size_t size) {
    if (idx->count >= idx->capacity) {
        idx->capacity *= 2;
        idx->array = realloc(idx->array, sizeof(indexbook) * idx->capacity);
    }
    idx->array[idx->count].key = key;
    idx->array[idx->count].offset = offset;
    idx->array[idx->count].size = size;
    idx->count++;
}


int index_find(Index *idx, int key) {
    /*for (int i = 0; i < idx->count; i++) {
        if (idx->array[i].key == key)
            return i;
    }*/

    int middle;
    int loop = 0;

    if (idx == NULL )
    {
        fprintf(stdout, "Record with bookId=%i does not exist", key);
        return -1;
    }

    middle = (idx->count / 2);

    while(loop == 0)
    {
        if (middle == idx->count || middle == 0)
        {
            loop = 1;
            fprintf(stdout, "Record with bookId=%i does not exist", key);
        }
        else if (idx->array[middle].key == key )
        {
            /*prindInd(parametro1, parametro2)*/
            fprintf(stdout, "%i|Isbn_I_dont_know_how_to_get|Name_I_dont_know_how_to_get|mark_I_do_not_know_how_to_get", key);
            loop = 1;
        }
        else if (idx->array[middle].key < key)
        {
            middle = (middle + idx->count) / 2;
        }
        else{
            middle = (0 + middle) / 2;
        }

    
        
    }

    
    







    return -1;
}

void index_load(Index *idx, const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return;

    fread(&idx->count, sizeof(int), 1, f);
    idx->capacity = idx->count > 0 ? idx->count : 10;
    idx->array = realloc(idx->array, sizeof(indexbook) * idx->capacity);

    fread(idx->array, sizeof(indexbook), idx->count, f);

    fclose(f);
}

void index_save(Index *idx, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;

    fwrite(&idx->count, sizeof(int), 1, f);
    fwrite(idx->array, sizeof(indexbook), idx->count, f);

    fclose(f);
}






void printInd(indexbook AAA){
    fprintf(stdout, "Entry #X\n");
    fprintf(stdout, "   key: #%i", AAA.key);
    fprintf(stdout, "   offset: #%i", AAA.offset);
    fprintf(stdout, "   size: #%i", AAA.size);
}

void printLst(indexbook lstBORRADOS){
    
}

void printRec(indexbook lstAAAAA){
}