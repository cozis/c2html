// Build with: gcc example.c c2html.c -o example -Wall -Wextra
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "c2html.h"

int main()
{
    // Table mode refers to the structure of the output HTML.
    // If table_mode is turned off, then the output lines are
    // separated by <br /> tags and there are no line numbers.
    // Using table mode, then the lines are represented as rows
    // of an HTML. The table has a first column with the line
    // numbers and the second with their content.
    _Bool table_mode = 0;
    const char *prefix = NULL;

    char *c = 
      "int main() {\n"
      "  int a = 5;\n"
      "  return 0;\n"
      "}\n";

    char *html = c2html(c, strlen(c), table_mode, prefix, NULL);
    printf("%s\n", html);
    free(html);
    return 0;
}