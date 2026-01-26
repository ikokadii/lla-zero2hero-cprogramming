#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
    if (NULL == dbhdr) {
        return;
    }
    if (NULL == employees) {
        return;
    }

    int i = 0;
    for (; i < dbhdr->count; i++) {
        printf("employee_%d: \n", i);
        printf("\t name: %s\n", employees[i].name);
        printf("\t address: %s\n", employees[i].address);
        printf("\t hours: %d\n", employees[i].hours);
    }
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {
    if (NULL == dbhdr) {
        return STATUS_ERROR;
    }
    if (NULL == employees) {
        return STATUS_ERROR;
    }
    if (NULL == *employees) {
        return STATUS_ERROR;
    }
    if (NULL == addstring) {
        return STATUS_ERROR;
    }

    char *name = strtok(addstring, ",");
    if (NULL == name) {
        return STATUS_ERROR;
    }

    char *address = strtok(NULL, ",");
    if (NULL == address) {
        return STATUS_ERROR;
    }

    char *hours = strtok(NULL, ",");
    if (NULL == hours) {
        return STATUS_ERROR;
    }

    struct employee_t *e = *employees;
    e = realloc(e, sizeof(struct employee_t) * (dbhdr->count + 1));
    if (NULL == e) {
        perror("realloc");
        return STATUS_ERROR;
    }

    dbhdr->count++;
    strncpy(e[dbhdr->count - 1].name, name, sizeof(e[dbhdr->count - 1].name)-1);
    strncpy(e[dbhdr->count - 1].address, address, sizeof(e[dbhdr->count - 1].address)-1);
    e[dbhdr->count - 1].hours = atoi(hours);
    *employees = e;
    return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
    if (fd < 0) {
        printf("invalid fd\n");
        return STATUS_ERROR;
    }

    int count = dbhdr->count;
    int size_read = -1;

    struct employee_t *employees = calloc(count, sizeof(struct employee_t));
    if (employees == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    size_read = read(fd, employees, count * sizeof(struct employee_t));
    if (size_read != (count * sizeof(struct employee_t))) {
        printf("failed to read employees\n");
        free(employees);
        employees = NULL;
        return STATUS_ERROR;
    }

    for (int i = 0; i < count; i++) {
        employees[i].hours = ntohl(employees[i].hours);
    }

    *employeesOut = employees;
    return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    int size_written = -1;

    if (fd < 0) {
        printf("invalid fd\n");
        return STATUS_ERROR;
    }

    // count before htonl
    int count_hostlong = dbhdr->count;

    dbhdr->magic = htonl(dbhdr->magic);
    dbhdr->version = htons(dbhdr->version);
    dbhdr->count = htons(dbhdr->count);
    // dbhdr->filesize = htonl(dbhdr->filesize);
    dbhdr->filesize = htonl(sizeof(struct dbheader_t) + count_hostlong * sizeof(struct employee_t));

    lseek(fd, 0, SEEK_SET);
    size_written = write(fd, dbhdr, sizeof(struct dbheader_t));
    if (size_written != sizeof(struct dbheader_t)) {
        perror("write header");
        return STATUS_ERROR;
    }

    for (int i = 0; i < count_hostlong; i++) {
        employees[i].hours = htonl(employees[i].hours);
        size_written = write(fd, &employees[i], sizeof(struct employee_t));
        if (size_written != sizeof(struct employee_t)) {
            perror("write employee");
            return STATUS_ERROR;
        }
    }
    return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
    if (fd < 0) {
        printf("invalid fd\n");
        return STATUS_ERROR;
    }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    int size_read = read(fd, header, sizeof(struct dbheader_t));
    if (size_read != sizeof(struct dbheader_t)) {
        perror("read");
        free(header);
        return STATUS_ERROR;
    }

    header->magic = ntohl(header->magic);
    header->version = ntohs(header->version);
    header->count = ntohs(header->count);
    header->filesize = ntohl(header->filesize);

    if (header->magic != HEADER_MAGIC) {
        printf("improper header magic\n");
        free(header);
        return STATUS_ERROR;
    }

    if (header->version != 1) {
        printf("improper header verion\n");
        free(header);
        return STATUS_ERROR;
    }

    struct stat database_stat = {0};
    fstat(fd, &database_stat);
    if (header->filesize != database_stat.st_size) {
        printf("corrupted database\n");
        free(header);
        return STATUS_ERROR;
    }

    *headerOut = header;
    return STATUS_SUCCESS;
}

//int create_db_header(int fd, struct dbheader_t **headerOut) {
int create_db_header(struct dbheader_t **headerOut) {
    // if (fd < 0) {
    //     printf("invalid fd\n");
    //     return STATUS_ERROR;
    // }

    struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
    if (header == NULL) {
        perror("calloc");
        return STATUS_ERROR;
    }

    header->count = 0;
    header->filesize = sizeof(struct dbheader_t);
    header->magic = HEADER_MAGIC;
    header->version = 0x1;

    *headerOut = header;

    return STATUS_SUCCESS;
}
