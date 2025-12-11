#include "record.h"

#include "utils.h"

size_t record_compute_size(BookRecord* rec) {
        size_t title_len = strlen(rec->title);
        size_t printed_len = strlen(rec->printedBy);

        return sizeof(int) + 16 + title_len + printed_len;
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

        /* title */
        if (fwrite(rec->title, 1, strlen(rec->title), f) != strlen(rec->title)) return ERROR;

        /* printedBy (sin '|') */
        if (fwrite(rec->printedBy, 1, strlen(rec->printedBy), f) != strlen(rec->printedBy)) return ERROR;

        return OK;
}

BookRecord* record_read(FILE* f, long int offset) {
        if (!f) return NULL;
        fseek(f, offset, SEEK_SET);

        size_t size;
        fread(&size, sizeof(size_t), 1, f);

        BookRecord* rec = malloc(sizeof(BookRecord));
        fread(&rec->bookID, sizeof(int), 1, f);

        fread(rec->isbn, 1, 16, f);

        /*fgets(rec->title, 129, f);
        fgets(rec->printedBy, 129, f);*/
        size_t remaining = 0;
        if (size > (sizeof(int) + 16)) remaining = size - (sizeof(int) + 16);

        char* buf = NULL;
        if (remaining > 0) {
                buf = malloc(remaining + 1);
                if (!buf) {
                        free(rec);
                        return NULL;
                }
                if (fread(buf, 1, remaining, f) != remaining) {
                        free(buf);
                        free(rec);
                        return NULL;
                }
                buf[remaining] = '\0';

                char* delim = strchr(buf, '|');
                if (delim) {
                        size_t title_len = (size_t)(delim - buf + 1);

                        if (title_len >= sizeof(rec->title)) title_len = sizeof(rec->title) - 1;
                        memcpy(rec->title, buf, title_len);
                        rec->title[title_len] = '\0';

                        size_t printed_len = remaining - (size_t)(delim - buf + 1);
                        if (printed_len >= sizeof(rec->printedBy)) printed_len = sizeof(rec->printedBy) - 1;
                        if (printed_len > 0) memcpy(rec->printedBy, buf + title_len, printed_len);
                        rec->printedBy[printed_len] = '\0';
                } else {
                        /* no delimiter found: put all into title and keep printedBy empty */
                        size_t copy_len = remaining;
                        if (copy_len >= sizeof(rec->title)) copy_len = sizeof(rec->title) - 1;
                        memcpy(rec->title, buf, copy_len);
                        rec->title[copy_len] = '\0';
                        rec->printedBy[0] = '\0';
                }
                free(buf);
        } /*else {
                 no variable part
                rec->title[0] = '\0';
                rec->printedBy[0] = '\0';
        }*/
        return rec;
}