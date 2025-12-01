#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING 256

typedef enum { OK, ERROR } Status;

char* read_line();

#endif