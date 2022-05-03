#include <assert.h>
#include <stdio.h>
#include "c2html.h"

int main(int argc, char **argv)
{
    assert(argc > 0);
    if(argc < 3) {
        fprintf(stderr, 
            "Missing input ad output files\n"
            "\n"
            "Usage:\n"
            "  $ %s input.c output.html\n"
            "\n", argv[0]);
        return -1;
    }

    const char *err;

    err = c2html(argv[1], argv[2]);
    if(err != NULL)
        fprintf(stderr, "ERROR: %s\n", err);
    fprintf(stderr, "OK");
    return err == NULL ? -1 : 0;
}