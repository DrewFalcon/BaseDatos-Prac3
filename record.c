#include "record.h"

#include "utils.h"
size_t record_compute_size(BookRecord* rec) {
        return sizeof(int)               /* bookID */
               + 16                      /* ISBN: EXACTAMENTE 16 bytes */
               + strlen(rec->title) + 1  /* title + '|' */
               + strlen(rec->printedBy); /* printedBy SIN '|' */
}
Status record_write(FILE* f, BookRecord* rec) {
        if (!f || !rec) return ERROR;

        size_t size = record_compute_size(rec);

        /* escribir tamaño */
        if (fwrite(&size, sizeof(size_t), 1, f) != 1) return ERROR;

        /* bookID */
        if (fwrite(&rec->bookID, sizeof(int), 1, f) != 1) return ERROR;

        /* ISBN: EXACTAMENTE 16 bytes */
        if (fwrite(rec->isbn, 1, 16, f) != 16) return ERROR;

        /* title + '|' */
        if (fwrite(rec->title, 1, strlen(rec->title), f) != strlen(rec->title)) return ERROR;
        if (fwrite("|", 1, 1, f) != 1) return ERROR;

        /* printedBy (sin '|') */
        if (fwrite(rec->printedBy, 1, strlen(rec->printedBy), f) != strlen(rec->printedBy)) return ERROR;

        return OK;
}

BookRecord* record_read(FILE* f, long int offset) {
        fseek(f, offset, SEEK_SET);

        size_t size;
        fread(&size, sizeof(size_t), 1, f);

        BookRecord* rec = malloc(sizeof(BookRecord));
        fread(&rec->bookID, sizeof(int), 1, f);
        fread(rec->isbn, 1, 16, f);

        fgets(rec->title, 129, f);
        fgets(rec->printedBy, 129, f);
        return rec;
}