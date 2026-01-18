#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "common.h"

// create a new dbfile.
int create_db_file(char *filename) {
    int status_file_close = -1;

    if (filename == NULL) {
        return STATUS_ERROR;
    }

    // check if the file already exists.
    int fd = open(filename, O_RDONLY);
    if (fd != -1) {
        printf("file already exists. filepath=%s\n", filename);
        status_file_close = close(fd);
        if (status_file_close == -1) {
            perror("close");
            return STATUS_ERROR;
        }
        return STATUS_ERROR;
    }

    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("open");
        return STATUS_ERROR;
    }

    // status_file_close = close(fd);
    // if (status_file_close == -1) {
    //     perror("close");
    //     return STATUS_ERROR;
    // }

    printf("file created. filepath=%s\n", filename);
    return fd;
}

// open a dbfile.
int open_db_file(char *filename) {
    int fd = -1;
    fd = open(filename, O_RDWR);
    if (fd == -1) {
        perror("open");
        return STATUS_ERROR;
    }

    return fd;
}

// close a dbfile.
int close_db_file(int dbfd) {
    if (dbfd < 0) {
        printf("db file is already closed.\n");
        return STATUS_ERROR;
    }

    int status_file_close = close(dbfd);
    if (status_file_close == -1) {
        perror("close");
        return STATUS_ERROR;
    }

    return STATUS_SUCCESS;
}
