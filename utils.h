#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* read_line();
void split_record(char *input, BookRecord *rec);

#endif