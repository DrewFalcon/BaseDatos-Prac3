#include "record.h"

size_t record_compute_size(BookRecord *rec) {
    return sizeof(int) + 16 + strlen(rec->title) + 1 + strlen(rec->printedBy) + 1;
}

void record_write(FILE *f, BookRecord *rec) {
    size_t size = record_compute_size(rec);
    fwrite(&size, sizeof(size_t), 1, f);
    fwrite(&rec->bookID, sizeof(int), 1, f);
    fwrite(rec->isbn, 1, 16, f);
    fwrite(rec->title, 1, strlen(rec->title) + 1, f);
    fwrite(rec->printedBy, 1, strlen(rec->printedBy) + 1, f);
}

BookRecord* record_read(FILE *f, long int offset) {
    fseek(f, offset, SEEK_SET);

    size_t size;
    fread(&size, sizeof(size_t), 1, f);

    BookRecord *rec = malloc(sizeof(BookRecord));
    fread(&rec->bookID, sizeof(int), 1, f);
    fread(rec->isbn, 1, 16, f);

    fgets(rec->title, 129, f);
    fgets(rec->printedBy, 129, f);
    return rec;
}