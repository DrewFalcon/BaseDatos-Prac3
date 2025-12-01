#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deleted.h"
#include "index.h"
#include "record.h"
#include "utils.h"

#define MAX_PRINTED_BY 1000
#define MAX_TITLE 1000
/* Private Functions */
Status add(FILE* db, Index* index, DeletedList* deleted, char* arguments, int strategy);
Status find(FILE* db, Index* index, char* arguments);
Status del(FILE* db, Index* index, DeletedList* deleted, char* arguments);
Status printInd(Index* index);
Status printRec(FILE* db, Index* index);
Status printLst(DeletedList* deleted);

/* Auxiliary functions */

int main(int argc, char** argv) {
        int strategy = 0;
        char* root_name;
        char db_filename[MAX_STRING], ind_filename[MAX_STRING], lst_filename[MAX_STRING];

        Index* index = NULL;
        DeletedList* deleted = NULL;
        FILE* db = NULL;

        /* Argument checking */
        if (argc < 3) {
                printf("Usage: %s <bestfit|firstfit|worstfit> <basename>\n", argv[0]);
                return -1;
        }

        if (strcmp(argv[1], "bestfit") == 0)
                strategy = BESTFIT;
        else if (strcmp(argv[1], "firstfit") == 0)
                strategy = FIRSTFIT;
        else if (strcmp(argv[1], "worstfit") == 0)
                strategy = WORSTFIT;
        else {
                printf("Unknown search strategy %s\n", argv[1]);
                return -1;
        }

        root_name = argv[2];

        /* Build filenames */
        sprintf(db_filename, "%s.db", root_name);
        sprintf(ind_filename, "%s.ind", root_name);
        sprintf(lst_filename, "%s.lst", root_name);

        /* Open DB file (correct way) */
        db = fopen(db_filename, "r+b");
        if (!db) db = fopen(db_filename, "w+b");
        if (!db) {
                printf("Error opening DB file\n");
                return -1;
        }

        /* Load index and deleted list */
        index = index_create();
        index_load(index, ind_filename);

        deleted = deleted_create(strategy);
        deleted_load(deleted, lst_filename);

        printf("Library ready. Enter commands:\n");

        while (1) {
                printf("> ");
                char* line = read_line();
                if (!line) break;

                char* cmd = strtok(line, " ");
                char* args = strtok(NULL, "");

                if (!cmd) {
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "exit") == 0) {
                        free(line);
                        break;
                }

                if (strcmp(cmd, "add") == 0) {
                        add(db, index, deleted, args, strategy);
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "find") == 0) {
                        find(db, index, args);
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "del") == 0) {
                        del(db, index, deleted, args);
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "printRec") == 0) {
                        printRec(db, index);
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "printInd") == 0) {
                        printInd(index);
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "printLst") == 0) {
                        printLst(deleted);
                        free(line);
                        continue;
                }

                printf("Unknown command\n");
                free(line);
        }

        /* Save structures */
        index_save(index, ind_filename);
        deleted_save(deleted, lst_filename);

        index_free(index);
        deleted_free(deleted);
        fclose(db);

        return 0;
}

/* Empty implementations */
Status add(FILE* db, Index* index, DeletedList* deleted, char* arguments, int strategy) {
        BookRecord* rec = NULL;
        char* token = NULL;
        size_t tlen = 0;
        size_t size = 0;
        long int offset = 0;

        if (!arguments) {
                printf("Uso correcto: add bookID|ISBN|Title|Printed_by\n");
                return ERROR;
        }

        /* Reservar registro */
        rec = malloc(sizeof(BookRecord));
        if (!rec) {
                printf("Malloc failed\n");
                return ERROR;
        }
        /* Inicializar campos por seguridad */
        memset(rec, 0, sizeof(BookRecord));

        /* Parsear argumentos */
        token = strtok(arguments, "|");
        if (!token) {
                printf("Formato inválido\n");
                free(rec);
                return ERROR;
        }
        rec->bookID = atoi(token);

        token = strtok(NULL, "|");
        if (!token) {
                printf("Formato inválido\n");
                free(rec);
                return ERROR;
        }
        /* Copiar ISBN de forma segura: rellenar hasta 16 bytes */
        memset(rec->isbn, 0, 16);
        tlen = strlen(token);
        if (tlen > 16) {
                tlen = 16;
        }
        memcpy(rec->isbn, token, tlen);

        token = strtok(NULL, "|");
        if (!token) {
                printf("Formato inválido\n");
                free(rec);
                return ERROR;
        }
        strcpy(rec->title, token);

        token = strtok(NULL, "");
        if (!token) {
                printf("Formato inválido\n");
                free(rec);
                return ERROR;
        }
        strncpy(rec->printedBy, token, MAX_PRINTED_BY - 1);
        rec->printedBy[MAX_PRINTED_BY - 1] = '\0';

        /* 1) comprobar duplicado */
        if (index_find(index, rec->bookID) != -1) {
                printf("Record with BookID=%d exists.\n", rec->bookID);
                free(rec);
                return ERROR;
        }

        /* 2) calcular tamaño correcto */
        size = record_compute_size(rec); /* <-- PASAR rec, no &rec */

        /* 3) buscar hueco */
        offset = deleted_find_fit(deleted, size);

        /* 4) si no hay hueco, append al final */
        if (offset < 0) {
                if (fseek(db, 0, SEEK_END) != 0) {
                        printf("fseek failed\n");
                        free(rec);
                        return ERROR;
                }
                offset = ftell(db);
                if (offset < 0) {
                        printf("ftell failed\n");
                        free(rec);
                        return ERROR;
                }
        } else {
                /* deleted_find_fit ya actualizó la lista (eliminó o redujo el hueco) */
                if (fseek(db, offset, SEEK_SET) != 0) {
                        printf("fseek failed\n");
                        free(rec);
                        return ERROR;
                }
        }

        /* 5) escribir registro en disco (record_write maneja el formato binario) */
        if (record_write(db, rec) == ERROR) {
                printf("Error escribiendo registro\n");
                free(rec);
                return ERROR;
        }

        /* 6) actualizar índice */

        index_insert(index, rec->bookID, offset, size);

        printf("Record with BookID=%d has been added to the database\n", rec->bookID);

        free(rec);
        return OK;
}

Status find(FILE* db, Index* index, char* arguments) { return OK; }
Status del(FILE* db, Index* index, DeletedList* deleted, char* arguments) {
        int bookID = 0;
        int found_index = 0;
        if (!db || !index || !deleted) {
                return ERROR;
        }

        /* Parsear argumentos */
        bookID = atoi(arguments);

        found_index = index_find(index, bookID);
        if (found_index == -1) {
                printf("Record with bookId=%i does not exist", bookID);
                return ERROR;
        } else {
                deleted_insert(deleted, index->array[found_index].size, index->array[found_index].offset);
                index_delete(index, bookID);
                printf("Record with bookId=%i has been deleted", bookID);
        }

        return OK;
}
Status printInd(Index* index) { return OK; }
Status printRec(FILE* db, Index* index) { return OK; }
Status printLst(DeletedList* deleted) { return OK; }