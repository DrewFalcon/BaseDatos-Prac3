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
Status add(FILE* db, Index* index, DeletedList* deleted, char* arguments);
Status del(FILE* db, Index* index, DeletedList* deleted, char* arguments);
Status find(FILE* db, Index* index, char* arguments);
Status printInd(Index* lstInd);
Status printRec(Index* lstRec, FILE* data_file);
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
                printf("Usage: %s <best_fit|first_fit|worst_fit> <basename>\n", argv[0]);
                return -1;
        }

        if (strcmp(argv[1], "best_fit") == 0)
                strategy = BESTFIT;
        else if (strcmp(argv[1], "first_fit") == 0)
                strategy = FIRSTFIT;
        else if (strcmp(argv[1], "worst_fit") == 0)
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

        /*TEMPORARY CHANGE IN INTERFACE*/
        /*printf("Library ready. Enter commands:\n");*/
        printf("Type command and argument/s.\n");

        while (1) {
                /*TEMPORARY CHANGE IN INTERFACE*/
                /*printf("> ");*/

                fprintf(stdout, "exit\n");
                fflush(stdout);


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
                        add(db, index, deleted, args);
                        //printf("exit\n");
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "find") == 0) {
                        find(db, index, args);
                        //printf("exit\n");
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "del") == 0) {
                        del(db, index, deleted, args);
                        //printf("exit\n");
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "printRec") == 0) {
                        printRec(index, db);
                        //printf("exit\n");
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "printInd") == 0) {
                        printInd(index);
                        //printf("exit\n");
                        free(line);
                        continue;
                }

                if (strcmp(cmd, "printLst") == 0) {
                        printLst(deleted);
                        //free(line);
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
Status add(FILE* db, Index* index, DeletedList* deleted, char* arguments) {
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
        strcat(rec->isbn, "\0");
        /*printf("1ISBN guardado: %s\n", rec->isbn);*/

        token = strtok(NULL, "|");
        /*printf("Token leido para el titulo: %s\n", token);*/
        if (!token) {
                printf("Formato inválido\n");
                free(rec);
                return ERROR;
        }
        strcpy(rec->title, token);
        strcat(rec->title, "|");
        /*printf("Titulo puesto %s\n", rec->title);
        printf("2ISBN guardado: %s\n", rec->isbn);*/

        token = strtok(NULL, "\0");
        if (!token) {
                printf("Formato inválido\n");
                free(rec);
                return ERROR;
        }
        /*Im modifying this ONLY TEMPORARILY:*/
        strncpy(rec->printedBy, token, 128 - 1);
        rec->printedBy[128 - 1] = '\0';

        /* 1) comprobar duplicado */
        if (index_find(index, rec->bookID) >= 0) {
                printf("Record with BookID=%d exists.\n", rec->bookID);
                free(rec);
                return ERROR;
        }

        /* 2) calcular tamaño correcto */
        size = record_compute_size(rec);

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
        /*printf("Datos añadidos: BookID: %d, ISBN: %s, Titulo: %s, Editorial: %s", rec->bookID, rec->isbn, rec->title,
               rec->printedBy);*/
        free(rec);
        return OK;
}

Status del(FILE* db, Index* index, DeletedList* deleted, char* arguments) {
        int bookID = 0;
        int found_index = 0;
        if (!db || !index || !deleted) {
                return ERROR;
        }

        /* Parsear argumentos */
        bookID = atoi(arguments);

        found_index = index_find(index, atoi(arguments));
        if (found_index <= -1) {
                printf("Item with key %i does not exist\n", bookID);
                return ERROR;
        } else {
                deleted_insert(deleted, index->array[found_index].size, index->array[found_index].offset);
                index_delete(index, bookID);
                printf("Record with BookID=%i has been deleted\n", bookID);
        }

        return OK;
}

Status find(FILE* db, Index* index, char* arguments) {
        int found_index = 0;

        if (db == NULL || index == NULL || arguments == NULL) {
                return ERROR;
        }

        found_index = index_find(index, atoi(arguments));
        index_print(index, db, atoi(arguments), found_index);

        return OK;
}

/* Imprime el contenido del Índice tal y como está ordenado en memoria.*/
Status printInd(Index* lstInd) {
        int i = 0;

        if (!lstInd) {
                return ERROR;
        }

        for (i = 0; i < lstInd->count; i++) {
                fprintf(stdout, "Entry #%i\n", i);
                fprintf(stdout, "    key: #%i\n", lstInd->array[i].key);
                fprintf(stdout, "    offset: #%li\n", lstInd->array[i].offset);
                fprintf(stdout, "    size: #%zu\n", lstInd->array[i].size);
        }

        return OK;
}

Status printLst(DeletedList* lst) {
        int i;

        if (!lst) {
                return ERROR;
        }

        for (i = 0; i < lst->count; i++) {
                printf("Entry #%i\n", i );
                printf("    offset: #%ld\n", lst->array[i].offset);
                printf("    size: #%zu\n", lst->array[i].size);
        }
        return OK;
}

/*Imprime el contenido del fichero que contiene los libros siguiendo el orden
  marcado por el Índice e ignorando los registros borrados. */
Status printRec(Index* lstRec, FILE* data_file) {
        if (!lstRec || !data_file) {
                return ERROR;
        }
        /*BookID | ISBN | Titulo | Editorial*/
        for (int i = 0; i < lstRec->count; i++) {
                /* Leer registro del archivo usando el offset del índice*/
                BookRecord* rec = record_read(data_file, lstRec->array[i].offset);
                if (rec != NULL) {
                        printf("%d|%s|%s%s\n", rec->bookID, rec->isbn, rec->title, rec->printedBy);
                        free(rec); /* Liberar memoria*/
                }
        }

        return OK;
}