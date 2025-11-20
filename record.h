#ifndef RECORD_H
#define RECORD_H

#include <stdio.h>
#include <stdlib.h>

// Estructura interna del libro
typedef struct {
    int bookID;
    char isbn[16];
    char title[129];      // +1 para '\0'
    char printedBy[129];
} BookRecord;

// Funciones mínimas
size_t record_compute_size(BookRecord *rec);
void record_write(FILE *f, BookRecord *rec);
BookRecord* record_read(FILE *f, long int offset);

#endif