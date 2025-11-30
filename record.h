#ifndef RECORD_H
#define RECORD_H

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

// Estructura interna del libro
typedef struct {
        int bookID;
        char isbn[16];
        char title[128];
        char printedBy[128];
} BookRecord;

// Funciones mínimas
size_t record_compute_size(BookRecord* rec);
Status record_write(FILE* f, BookRecord* rec);
BookRecord* record_read(FILE* f, long int offset);

#endif