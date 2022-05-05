#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "c2html.h"

static char *load_file(const char *file, long *size)
{
    FILE *fp = fopen(file, "rb");
    if(fp == NULL)
        return NULL;

    fseek(fp, 0, SEEK_END);
    long size2 = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *data = malloc(size2+1);
    if(data == NULL) {
        fclose(fp);
        return NULL;
    }

    fread(data, 1, size2, fp);
    fclose(fp);

    if(size)
        *size = size2;

    data[size2] = '\0';
    return data;
}

int main(int argc, char **argv)
{
    if(argc < 3) {
        fprintf(stderr, "ERROR: Missing input and output file\n");
        return -1;
    }
    char *src_file = argv[1];
    char *dst_file = argv[2];

    long  src_size;
    char *src_data = load_file(src_file, &src_size);

    if(src_data == NULL) {
        fprintf(stderr, "ERROR: Failed to open \"%s\"\n", src_file);
        return -1;
    }

    _Bool table_mode = 0;
    const char *class_prefix = "c2h-";

    const char *error;
    char *dst_data = c2html(src_data, src_size, table_mode, class_prefix, &error);

    if(dst_data == NULL)
        fprintf(stderr, "ERROR: %s\n", error);
    else {

        FILE *fp = fopen(dst_file, "wb");
        if(fp == NULL)
            fprintf(stderr, "ERROR: Failed to open \"%s\"\n", dst_file);
        else {
            fwrite(dst_data, 1, strlen(dst_data), fp);
            fclose(fp);
            fprintf(stderr, "OK\n");
        }

        free(dst_data);
    }

    free(src_data);
    return 0;
}