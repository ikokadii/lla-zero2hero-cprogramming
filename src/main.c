#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n - create new database file\n");
    printf("\t -f - (required) path to database file\n");
    return;
}

int main(int argc, char *argv[]) {
    bool newfile = false;
    char *filepath = NULL;
    int status_file_create = -1;
    int status_file_close = -1;
    int status_db_header = -1;
    struct dbheader_t *dbheader = NULL;

    int dbfd = -1;

    // getopt. "n"(new) and "f"(filename).
    // "nf:" ... "f" option requires an argument. (the text is in optarg.)
    int opt = 0;
    while ((opt = getopt(argc, argv, "nf:")) != -1) {
        switch (opt) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case '?':
                printf("Unknown option -%c\n", opt);
                break;
            default:
                return -1;
        }
    }

    if (filepath == NULL) {
        printf("Filepath is a require argument.\n");
        print_usage(argv);
        return 0;
    }

    // printf("newfile: %d\n", newfile);
    // printf("filepath: %s\n", filepath);

    // create a file if newfile.
    if (newfile == true) {
        status_file_create = create_db_file(filepath);
        if (status_file_create == STATUS_ERROR) {
            printf("failed to create a new dbfile. filepath=%s\n", filepath);
            return -1;
        } else {
            // printf("[created] filepath=%s\n", filepath);
        }
    }

    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
        printf("failed to open a dbfile. filepath=%s\n", filepath);
        return -1;
    } else {
        // printf("[opened] filepath=%s\n", filepath);
    }

    if (newfile == true) {
        // status_db_header = create_db_header(dbfd, &dbheader);
        status_db_header = create_db_header(&dbheader);
        if (status_db_header == STATUS_ERROR) {
            printf("failed to create database header\n");
            return -1;
        }
    } else {
        status_db_header = validate_db_header(dbfd, &dbheader);
        if (status_db_header == STATUS_ERROR) {
            printf("failed to validate database header\n");
            return -1;
        }
    }

    status_db_header = output_file(dbfd, dbheader, NULL);
    if (status_db_header == STATUS_ERROR) {
        free(dbheader);
        printf("failed to write database header\n");
        return -1;
    }

    status_file_close = close_db_file(dbfd);
    if (status_file_close == STATUS_ERROR) {
        free(dbheader);
        printf("failed to close a dbfile. filepath=%s\n", filepath);
        return -1;
    }
    // printf("[closed] filepath=%s\n", filepath);


    free(dbheader);
    dbheader = NULL;

    return 0;
}
