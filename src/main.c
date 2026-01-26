#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n - create new database file\n");
    printf("\t -f - (required) path to database file\n");
    printf("\t -l - list the employees\n");
    printf("\t -a - add via CSV list of (name,address,salary)\n");
    return;
}

void my_free(void **ptr) {
    if (ptr == NULL || *ptr == NULL) return;
    free(*ptr);
    *ptr = NULL;
}

int main(int argc, char *argv[]) {
    bool newfile = false;
    char *filepath = NULL;
    char *addstring = NULL;
    bool list = false;

    int status_file_create = -1;
    int status_file_close = -1;
    int status_create_header = -1;
    int status_validate_header = -1;
    int status_read_employee = -1;
    int status_add_employee = -1;
    int status_write_db = -1;
    struct dbheader_t *dbheader = NULL;
    struct employee_t *employees = NULL;
    int dbfd = -1;

    // getopt. "n"(new) and "f"(filename).
    // "nf:" ... "f" option requires an argument. (the text is in optarg.)
    int opt = 0;
    while ((opt = getopt(argc, argv, "nf:a:l")) != -1) {
        switch (opt) {
            case 'n':
                newfile = true;
                break;
            case 'f':
                filepath = optarg;
                break;
            case 'a':
                addstring = optarg;
                break;
            case 'l':
                list = true;
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
        dbfd = create_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("failed to create a new dbfile. filepath=%s\n", filepath);
            return -1;
        }

        status_create_header = create_db_header(&dbheader);
        if (status_create_header == STATUS_ERROR) {
            printf("failed to create database header\n");
            return -1;
        }
        // printf("new file: done\n");
    } else {
        dbfd = open_db_file(filepath);
        if (dbfd == STATUS_ERROR) {
            printf("failed to open a dbfile. filepath=%s\n", filepath);
            return -1;
        }

        status_validate_header = validate_db_header(dbfd, &dbheader);
        if (status_validate_header == STATUS_ERROR) {
            printf("failed to validate database header\n");
            return -1;
        }
        // printf("validate file: done\n");
    }

    // read employees
    status_read_employee = read_employees(dbfd, dbheader, &employees);
    if (status_read_employee == STATUS_ERROR) {
        // my_free((void **) &dbheader);
        free(dbheader);
        dbheader = NULL;
        return -1;
    }
    // printf("read employee: done\n");

    // add an employee
    if (addstring != NULL) {
        status_add_employee = add_employee(dbheader, &employees, addstring);
        if (status_add_employee ==  STATUS_ERROR) {
            // printf("failed to add employee\n");
            // my_free((void **) &dbheader);
            // my_free((void **) &employees);
            free(dbheader);
            dbheader = NULL;
            free(employees);
            employees = NULL;
            return -1;
        }

        // printf("add employee: done\n");
    }

    status_write_db = output_file(dbfd, dbheader, employees);
    if (status_write_db == STATUS_ERROR) {
        // my_free((void **) &dbheader);
        // my_free((void **) &employees);
        free(dbheader);
        dbheader = NULL;
        free(employees);
        employees = NULL;
        printf("failed to write database header\n");
        return -1;
    }
    // printf("write db: done\n");

    status_file_close = close_db_file(dbfd);
    if (status_file_close == STATUS_ERROR) {
        // my_free((void **) &dbheader);
        // my_free((void **) &employees);
        free(dbheader);
        dbheader = NULL;
        free(employees);
        employees = NULL;
        printf("failed to close a dbfile. filepath=%s\n", filepath);
        return -1;
    }
    // printf("close file: done\n");
    // printf("[closed] filepath=%s\n", filepath);


    // my_free((void **) dbheader);
    // my_free((void **) employees);
    free(dbheader);
    dbheader = NULL;
    free(employees);
    employees = NULL;
    // printf("all done\n");
    return 0;
}
