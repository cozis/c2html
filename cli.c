#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "c2html.h"

#ifdef C2H_TIMING
#include <time.h>
char *timed_c2html(const char *str, long len, 
                   const char *prefix, const char **error)
{
    if(str == NULL)
        str = "";

    if(len < 0)
        len = strlen(str);

    clock_t beg, end;
    char *res;

    beg = clock();
    res = c2html(str, len, prefix, error);
    end = clock();

    double time_spent = (double) (end - beg) 
                      / CLOCKS_PER_SEC;
    fprintf(stdout, 
        "-- Timing results ----\n"
        "Size: %ldb\n"
        "Time: %fs\n"
        "Rate: %.2f Mb/s\n"
        "----------------------\n", 
        len, time_spent, 
        (double) len/(time_spent * 1000000));
    return res;
}
#define c2html timed_c2html
#endif

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
                    const char *style_file, const char *prefix)
{
    if(input_file == NULL || output_file == NULL) {
        fprintf(stderr, 
            "Error: You must specify both input and output "
            "files using the --input and --output options\n");
        return -1;
    }

    if(prefix == NULL)
        prefix = "c2h-";

    /* Load the input string */
    long  input_size;
    char *input = load_file(input_file, &input_size);
    if(input == NULL) {
        fprintf(stderr, "Error: Failed to open file %s\n", input_file);
        return -1;
    }

    /* Convert it */
    const char *error;
    char *output = c2html(input, input_size, prefix, &error);
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

    /* All done! :^) */
    free(output);
    free(input);
    return 0;
}

static void print_help(FILE *fp, char *name) {
    fprintf(fp, 
        "\n"
        " c2html is a tool to add HTML syntax highlighting to C code.\n"
        "\n"
        " Basically you give  c2html  some C code  as input and  it classifies \n"
        " all the keywords, identifiers etc using <span> elements, associating \n"
        " them with the appropriate  class names. By applying a CSS stylesheet \n"
        " to the generated output, you get the highliting!\n"
        "\n"
        " The usage is:\n"
        "     $ %s -i file.c -o file.html [--style file.css] [-p <prefix>]\n" 
        "\n"
        " ..and here's a table of all available options:\n"
        "\n"
        "     -h,   --help            Show this message\n"
        "\n"
        "     -i,  --input file.c     File containing C code that needs\n"
        "                             to be converted\n"
        "\n"
        "     -o, --output file.html  HTML file where the output will be\n"
        "                             written to\n"
        "\n"
        "          --style style.css  CSS stylesheet that will be added\n"
        "                             to the HTML output\n"
        "\n"
        "     -p, --prefix <prefix>   The prefix of the HTML element's\n"
        "                             class names. The default is \"c2h-\"\n"
        "\n", name);
}

int main(int argc, char **argv)
{
    if(argc == 1) {
        print_help(stdout, argv[0]);
        return 0;
    }

    /* Parse command-line arguments */
    char *input_file = NULL, 
        *output_file = NULL,
         *style_file = NULL,
             *prefix = NULL;

    for(int i = 1; i < argc; i += 1) {
        if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {

            print_help(stdout, argv[0]);
            return -1;

        } else if(!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input")) {
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
        } else if(!strcmp(argv[i], "--style")) {
            i += 1;
            if(i == argc || argv[i][0] == '-') {
                fprintf(stderr, "Error: Missing argument after %s\n", argv[i-1]);
                return -1;
            }
            style_file = argv[i];
        } else {
            fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
            return -1;
        }
    }

    return fileconv(input_file, output_file, 
                    style_file, prefix);
}