#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database file>\n", argv[0]);
    printf("\t -n - create new database file\n");
    printf("\t -f - (required) path to database file\n");
    printf("    ");
    return;
}

int main(int argc, char *argv[]) {
    bool newfile = false;
    char *filepath = NULL;

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

    return 0;
}
