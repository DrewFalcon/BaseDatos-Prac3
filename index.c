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

void index_insert(Index* idx, int key, long int offset, size_t size) {
        int pos = 0;
        if (idx->count >= idx->capacity) {
                idx->capacity *= 2;
                idx->array = realloc(idx->array, sizeof(indexbook) * idx->capacity);
        }
        
        
        while (pos < idx->count && idx->array[pos].key < key )
                pos++;
        
        if (pos < idx->count)
        {
               memmove(&idx->array[pos + 1], /* destino */
                        &idx->array[pos],     /* origen */
                        (idx->count - pos) * sizeof(indexbook));
        }
        

        idx->array[pos].key = key;
        idx->array[pos].offset = offset;
        idx->array[pos].size = size;
        idx->count++;
        return;
}

void index_load(Index* idx, const char* filename) {
        FILE* f = fopen(filename, "rb");
        if (!f) return;

        /* Determine file size and derive number of entries.
           The control files use a 20-byte-per-entry layout (4B key,
           4B offset_lo, 4B offset_hi, 4B size, 4B padding) and no
           initial count. */
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (fsize <= 0) {
                fclose(f);
                return;
        }

        int entry_size = 20; /* bytes per entry in control format */
        int nentries = (int)(fsize / entry_size);

        idx->count = 0;
        idx->capacity = nentries > 0 ? nentries : 10;
        idx->array = realloc(idx->array, sizeof(indexbook) * idx->capacity);

        for (int i = 0; i < nentries; i++) {
                uint32_t key32, off_lo, off_hi, size32, pad;
                if (fread(&key32, sizeof(uint32_t), 1, f) != 1) break;
                if (fread(&off_lo, sizeof(uint32_t), 1, f) != 1) break;
                if (fread(&off_hi, sizeof(uint32_t), 1, f) != 1) break;
                if (fread(&size32, sizeof(uint32_t), 1, f) != 1) break;
                if (fread(&pad, sizeof(uint32_t), 1, f) != 1) break;

                idx->array[i].key = (int)key32;
                idx->array[i].offset = ((long int)off_hi << 32) | (long int)off_lo;
                idx->array[i].size = (size_t)size32;
                idx->count++;
        }

        fclose(f);
}

void index_save(Index* idx, const char* filename) {
        FILE* f = fopen(filename, "wb");
        if (!f) return;

        fwrite(&idx->count, sizeof(int), 1, f);
        fwrite(idx->array, sizeof(indexbook), idx->count, f);

         /*Write entries in the deterministic control format: for each
           entry write 5 uint32_t words: key, offset_lo, offset_hi, size, pad(0).
           This produces the exact binary layout used by the reference
           control files in this repo. 
        for (int i = 0; i < idx->count; i++) {
                uint32_t key32 = (uint32_t)idx->array[i].key;
                uint64_t off64 = (uint64_t)idx->array[i].offset;
                uint32_t off_lo = (uint32_t)(off64 & 0xFFFFFFFFu);
                uint32_t off_hi = (uint32_t)((off64 >> 32) & 0xFFFFFFFFu);
                uint32_t size32 = (uint32_t)idx->array[i].size;
                uint32_t pad = 0;

                fwrite(&key32, sizeof(uint32_t), 1, f);
                fwrite(&off_lo, sizeof(uint32_t), 1, f);
                fwrite(&off_hi, sizeof(uint32_t), 1, f);
                fwrite(&size32, sizeof(uint32_t), 1, f);
                fwrite(&pad, sizeof(uint32_t), 1, f);
        }*/

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

        if (idx != NULL && (idx->array) != NULL && pos >= 0 && pos <= idx->count) {
                rec = record_read(db, idx->array[pos].offset);
                if (rec != NULL && pos >= 0) {
                        fprintf(stdout, "%i|%s|%s%s\n", rec->bookID, rec->isbn, rec->title, rec->printedBy);
                        free(rec);
                }else{
                        fprintf(stdout, "Error accesing .db file with Offset");
                }
        }else{
                        fprintf(stdout, "Record with bookId=%i does not exist\n", key);
        }

        return;
}