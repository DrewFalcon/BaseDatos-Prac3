#include "utils.h"
#include "record.h"

char* read_line() {
    char buffer[1024];
    if (!fgets(buffer, 1024, stdin))
        return NULL;
    buffer[strcspn(buffer, "\n")] = 0;
    return strdup(buffer);
}

void split_record(char *input, BookRecord *rec) {
    char *token = strtok(input, "|");
    rec->bookID = atoi(token);

    token = strtok(NULL, "|");
    strcpy(rec->isbn, token);

    token = strtok(NULL, "|");
    strcpy(rec->title, token);

    token = strtok(NULL, "|");
    strcpy(rec->printedBy, token);
}