#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
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

static char *load_from_stream(FILE *fp, long *out_size, const char **err)
{
    char *data = NULL;
    long  capacity = 1 << 11;
    long  size = 0;

    while(1) {

        capacity *= 2;
        if((long) capacity < 0) {
            
            if(err) 
                *err = "Too big";
            
            free(data);
            return NULL;
        }

        void *temp = realloc(data, capacity);
        if(temp == NULL) {
            if(err)
                *err = "No memory";
            free(data);
            return NULL;
        }
        data = temp;

        long unused = capacity - size - 1; // Spare one byte for NULL termination.
        long num = fread(data + size, 1, unused, fp);
        size += num;

        if(num < unused) {
            // Either something went wrong or
            // we're done copying.
            if(ferror(fp)) {
                if(err)
                    *err = "Unknown read error";
                free(data);
                return NULL;
            }

            break;
        }
    }
    data[size] = '\0';

    if(out_size)
        *out_size = size;

    return data;
}

static char *load_file(const char *file, long *size)
{
    FILE *fp = fopen(file, "rb");
    if(fp == NULL)
        return NULL;
    char *data = load_from_stream(fp, size, NULL);
    fclose(fp);
    return data;
}

static long find_substr_or_end(const char *str, long len, long i, const char *substr)
{
    long substr_len = strlen(substr);

    while(i < len && (i + substr_len > len || 
          strncmp(str + i, substr, substr_len)))
        i += 1;

    return i;
}

static int tmplconv(FILE *in_fp, FILE *out_fp,
                    const char *prefix, 
                    const char *token_begin, 
                    const char *token_end)
{
    if(prefix == NULL)
        prefix = "c2h-";

    if(token_begin == NULL)
        token_begin = "<c2html>";

    if(token_end == NULL)
        token_end = "</c2html>";

    const char *err;

    long  input_size;
    char *input = load_from_stream(in_fp, &input_size, &err);
    if(input == NULL) {
        fprintf(stderr, "Error: Failed to read input (%s)\n", err);
        return -1;
    }

    long i = 0;
    while(1) {
        
        // Scan the substring that comes before the
        // next occurrence of the <c2html> token.
        long off = i;
        i = find_substr_or_end(input, input_size, i, token_begin);
        long len = i - off;

        {
            long written = fwrite(input + off, 1, len, out_fp);
            if(written < len) {
                fprintf(stderr, "Error: Failed to write to output\n");
                free(input);
                return -1;
            }
        }

        if(i == input_size)
            break;

        i += strlen(token_begin); // Consume the token.
        assert(i <= input_size);

        // Now get to the ending token.
        off = i;
        i = find_substr_or_end(input, input_size, i, token_end);
        len = i - off;

        {
            long  output_size;
            char *output = c2html(input + off, len, prefix, 
                                  &output_size, &err);
            if(output == NULL) {
                fprintf(stderr, "Error: %s\n", err);
                free(input);
                return -1;
            }

            long written = fwrite(output, 1, output_size, out_fp);
            
            free(output);

            if(written < len) {
                fprintf(stderr, "Error: Failed to write to output\n");
                free(input);
                return -1;
            }
        }

        if(i == input_size)
            break;

        i += strlen(token_end); // Consume the token.
        assert(i <= input_size);
    }

    free(input);
    return 0;
}

static int fileconv(FILE *in_fp, FILE *out_fp, 
                    const char *style_file, 
                    const char *prefix)
{
    if(prefix == NULL)
        prefix = "c2h-";

    const char *err;

    long  input_size;
    char *input = load_from_stream(in_fp, &input_size, &err);
    if(input == NULL) {
        fprintf(stderr, "Error: Failed to read input (%s)\n", err);
        return -1;
    }

    long  output_size;
    char *output = c2html(input, input_size, prefix, 
                          &output_size, &err);
    if(output == NULL) {
        fprintf(stderr, "Error: %s\n", err);
        free(input);
        return -1;
    }

    if(style_file != NULL) {

        char *style_data = load_file(style_file, NULL);
        if(style_data == NULL) {
            fprintf(stderr, "Error: Failed to open file %s\n", style_file);
            free(output);
            free(input);
            return -1;
        }

        bool failed = fputs("<style>",  out_fp) < 0
                   || fputs(style_data, out_fp) < 0
                   || fputs("</style>", out_fp) < 0;

        free(style_data);

        if(failed) {
            fprintf(stderr, "Error: Failed to write to output\n");
            free(output);
            free(input);
            return -1;
        }
    }

    long written = fwrite(output, 1, output_size, out_fp);

    free(output);
    free(input);

    if(written < output_size) {
        fprintf(stderr, "Error: Failed to write to output\n");
        return -1;
    }

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
        "     $ %s [-i file.c] [-o file.html] [--style file.css] [-p <prefix>] [-t [-s <token>] [-e <token>]]\n" 
        "\n"
        " ..and here's a table of all available options:\n"
        "\n"
        "     -h,   --help             Show this message\n"
        "\n"
        "     -i,  --input     file.c  File containing C code that needs\n"
        "                              to be converted\n"
        "\n"
        "     -o, --output  file.html  HTML file where the output will be\n"
        "                              written to\n"
        "\n"
        "          --style  style.css  CSS stylesheet that will be added\n"
        "                              to the HTML output\n"
        "\n"
        "     -p, --prefix   <prefix>  The prefix of the HTML element's\n"
        "                              class names. The default is \"c2h-\"\n"
        "\n"
        "     -t, --template           Only highlight the substrings of the\n"
        "                              input between the <c2html> and </c2html>\n"
        "                              tokens. The rest is copied unchanged.\n"
        "                              It's possible to change these tokens\n"
        "                              with --begin and --end.\n"
        "                              When using this mode, --style is ignored\n"
        "\n"
        "     -b, --begin    <string>  Specify the start of each substring to\n"
        "                              be converted. It only work when --template\n"
        "                              is also specified\n"
        "\n"
        "     -e, --end      <string>  Specify the end of each substring to be\n"
        "                              be converted. It only works when --template\n"
        "                              is also specified\n"
        "\n", name);
}

int main(int argc, char **argv)
{
    /* Parse command-line arguments */
    char *input_file = NULL, 
        *output_file = NULL,
         *style_file = NULL,
        *templ_begin = NULL,
          *templ_end = NULL,
             *prefix = NULL;
    bool    template = 0;

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
        } else if(!strcmp(argv[i], "-t") || !strcmp(argv[i], "--template")) {

            template = 1;

        } else if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--begin")) {

            i += 1;
            if(i == argc || argv[i][0] == '-') {
                fprintf(stderr, "Error: Missing argument after %s\n", argv[i-1]);
                return -1;
            }
            templ_begin = argv[i];

        } else if(!strcmp(argv[i], "-e") || !strcmp(argv[i], "--end")) {

            i += 1;
            if(i == argc || argv[i][0] == '-') {
                fprintf(stderr, "Error: Missing argument after %s\n", argv[i-1]);
                return -1;
            }
            templ_end = argv[i];

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

    bool use_stdin = (input_file == NULL);
    bool use_stdout = (output_file == NULL);
 
    FILE *in_fp, *out_fp;

    if(use_stdin)
        in_fp = stdin;
    else {
        in_fp = fopen(input_file, "rb");
        if(in_fp == NULL) {
            fprintf(stderr, "Error: Couldn't open file %s\n", input_file);
            return -1;
        }
    }

    if(output_file == NULL)
        out_fp = stdout;
    else {
        out_fp = fopen(output_file, "wb");
        if(out_fp == NULL) {
            if(!use_stdin)
                fclose(in_fp);
            fprintf(stderr, "Error: Couldn't open or create file %s\n", output_file);
            return -1;
        }
    }

    int rescode;
    if(template) {
        if(style_file != NULL)
            fprintf(stderr, "Warning: --style is ignored when using --template or -t\n");
        rescode = tmplconv(in_fp, out_fp, prefix, templ_begin, templ_end);
    } else {
        if(templ_begin != NULL || templ_end != NULL)
            fprintf(stderr, "Warning: --begin and --end are ignored when not using --template\n");
        rescode = fileconv(in_fp, out_fp, style_file, prefix);
    }

    if(!use_stdin) fclose(in_fp);
    if(!use_stdout) fclose(out_fp);
    return rescode;
}