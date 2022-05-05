#include <stdbool.h>
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

static int fileconv(const char *input_file, const char *output_file, 
                    const char *style_file, const char *prefix, bool notable)
{
    if(input_file == NULL || output_file == NULL) {
        fprintf(stderr, 
            "Error: You must specify both input and output "
            "files using the --input and --output options\n");
        return -1;
    }

    /* Load the input string */
    long  input_size;
    char *input = load_file(input_file, &input_size);
    if(input == NULL) {
        fprintf(stderr, "Error: Failed to open file %s\n", input_file);
        return -1;
    }

    /* Convert it */
    const char *error;
    char *output = c2html(input, input_size, 
                   !notable, prefix, &error);
    if(output == NULL) {
        fprintf(stderr, "Error: %s\n", error);
        free(input);
        return -1;
    }

    /* Open the output file */
    FILE *fp = fopen(output_file, "wb");
    if(fp == NULL) {
        fprintf(stderr, "ERROR: Failed to write to file %s\n", 
                output_file);
        free(output);
        free(input);
        return -1;
    }

    if(style_file != NULL) {
        char *style_data = load_file(style_file, NULL);
        if(style_data == NULL) {
            fprintf(stderr, "Error: Failed to open file %s\n", 
                    style_file);
            free(output);
            free(input);
            return -1;
        }
        fputs("<style>", fp);
        fputs(style_data, fp);
        fputs("</style>", fp);
        free(style_data);
    }

    /* ..and write the converted string to it */
    fwrite(output, 1, strlen(output), fp);
    fclose(fp);
    fprintf(stderr, "OK\n");

    /* All done! :^) */
    free(output);
    free(input);
    return 0;
}

int main(int argc, char **argv)
{

    /* Parse command-line arguments */
    char *input_file = NULL, 
        *output_file = NULL,
         *style_file = NULL,
             *prefix = NULL;
    bool     notable = 0;
    bool        http = 0;
    for(int i = 1; i < argc; i += 1) {
        if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input")) {
            i += 1;
            if(i == argc || argv[i][0] == '-') {
                fprintf(stderr, "Error: Missing argument after %s\n", argv[i-1]);
                return -1;
            }
            input_file = argv[i];
        } else if(!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
            i += 1;
            if(i == argc || argv[i][0] == '-') {
                fprintf(stderr, "Error: Missing argument after %s\n", argv[i-1]);
                return -1;
            }
            output_file = argv[i];
        } else if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--prefix")) {
            i += 1;
            if(i == argc || argv[i][0] == '-') {
                fprintf(stderr, "Error: Missing argument after %s\n", argv[i-1]);
                return -1;
            }
            prefix = argv[i];
        } else if(!strcmp(argv[i], "--no-table")) {
            notable = 1;
        } else if(!strcmp(argv[i], "--style")) {
            i += 1;
            if(i == argc || argv[i][0] == '-') {
                fprintf(stderr, "Error: Missing argument after %s\n", argv[i-1]);
                return -1;
            }
            style_file = argv[i];
        } else if(!strcmp(argv[i], "--http")) {
            http = 1;
        } else {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
            return -1;
        }
    }

    return fileconv(input_file, output_file, 
                    style_file, prefix, notable);
}