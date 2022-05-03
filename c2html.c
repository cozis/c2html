#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "c2html.h"

/* Converts files containing C code to an
 * HTML version with syntax highlighting.
 * The highlighting is best-effort, it's
 * only based on the tokenization result
 * and doesn't do a full parsing. Parsing
 * C would blow up the code of this utility.
 */

static _Bool iskeyword(const char *str)
{
    static const char *keywords[] = {
        "auto", "break", "case", "char",
        "const", "continue", "default",
        "do", "double", "else", "enum",
        "extern", "float", "for", "goto",
        "if", "int", "long", "register",
        "return", "short", "signed", 
        "sizeof", "static", "struct", 
        "switch", "typedef", "union",
        "unsigned", "void", "volatile",
        "while"
    };
    const int num_keywords = sizeof(keywords)/sizeof(keywords[0]);
    for(int i = 0; i < num_keywords; i += 1)
        if(!strcmp(keywords[i], str))
            return 1;
    return 0;
}

static _Bool isop(char c)
{
    static const char ops[] = ".<>=!+-*/%&^|;~?:"; 
    for(int i = 0; ops[i] != '\0'; i += 1)
        if(c == ops[i])
            return 1;
    return 0;
}

const char *c2html2(FILE *src, FILE *dst)
{
    char c = getc(src);
    
    while(isspace(c)) {
        if(c == ' ')
            fprintf(dst, " ");
        else if(c == '\n')
            fprintf(dst, "\n");

        c = getc(src);
    }

    while(c != EOF) {

        assert(!isspace(c));

        char hint = getc(src);
        fseek(src, -1, SEEK_CUR);

        if(c == '/' && (hint == '/' || hint == '*')) {

            _Bool mline = (hint == '*');
            char comment[1024];
            int len = 0;

            do {

                if(len == sizeof(comment)-1)
                    return "Static buffer is too small to "
                           "hold a comment this big";
                
                comment[len++] = c;
                c = getc(src);
                
                if(c == EOF)
                    break;
                
                if(!mline && c == '\n')
                    break;
                
                if(mline && c == '*') {

                    c = getc(src);

                    if(c == '/')
                        break;

                    if(len == sizeof(comment)-1)
                        return "Static buffer is too small to "
                           "hold a comment this big";
                    comment[len++] = '*';
                }

            } while(1);

            assert(len < (int) sizeof(comment));
            comment[len] = '\0';

            if(c == EOF)
                fprintf(dst, "%s", comment);
            else if(mline)
                fprintf(dst, "%s*/", comment);
            else
                fprintf(dst, "%s\n", comment);
        }

        if(c == '"' || c == '\'') {

            char str[512];
            int  len = 0;

            char f = c;
            c = getc(src);
            while(c != f && c != EOF) {
                if(len == sizeof(str)-1)
                    return "Static buffer is too small "
                           "to hold string literal";
                str[len++] = c;
                c = getc(src);
            }

            assert(len < (int) sizeof(str));
            str[len] = '\0';

            fprintf(dst, "<span class=\"vl-string\">%s</span>\n", str);

        } else if(isalpha(c) || c == '_') {

            char ident[128];
            int len = 0;

            do {
                if(len == sizeof(ident)-1)
                    return "Static buffer is too small "
                           "to hold identifier";
                ident[len++] = c;
                c = getc(src);
            } while(isalpha(c) || isdigit(c) || c == '_'); 

            assert(len < (int) sizeof(ident));
            ident[len] = '\0';

            if(iskeyword(ident))
                fprintf(dst, "<span class=\"keyword kw-%s\">%s</span>", 
                    ident, ident);
            else
                fprintf(dst, "<span class=\"ident\">%s</span>", ident);

        } else if(isdigit(c)) {

            char numb[64];
            int len = 0;

            do {
                if(len == sizeof(numb)-1)
                    return "Static buffer is too small to "
                           "hold a number with so many digits";
                numb[len++] = c;
                c = getc(src);
            } while(isdigit(c));

            // Here [c] refers to the first non-digit
            // character. We want to know the following
            // also.
            char hint = getc(src); // Read one.
            fseek(src, -1, SEEK_CUR); // Then go back by one.

            _Bool dot = 0;
            if(c == '.' && isdigit(hint)) {
                // Maybe it's a floating-point value?
                do {
                    if(len == sizeof(numb)-1)
                        return "Static buffer is too small to "
                               "hold a number with so many digits";
                    numb[len++] = c;
                    c = getc(src);
                } while(isdigit(c));
                dot = 1;
            }

            assert(len < (int) sizeof(numb));
            numb[len] = '\0';

            fprintf(dst, "<span class=\"%s\">%s</span>", 
                dot ? "vl-float" : "vl-int", numb);
        } else if(isop(c)) {
            
            char oper[32];
            int len = 0;

            do {
                if(len == sizeof(oper)-1)
                    return "Static buffer is too small to "
                           "hold an operator this long";
                oper[len++] = c;
                c = getc(src);
            } while(isop(c));
            
            assert(len < (int) sizeof(oper));
            oper[len] = '\0';

            fprintf(dst, "<span class=\"oper\">%s</span>", oper);
        } else {
            fprintf(dst, "%c", c);
            c = getc(src);
        }

        while(isspace(c)) {
            if(c == ' ')
                fprintf(dst, " ");
            else if(c == '\n')
                fprintf(dst, "\n");

            c = getc(src);
        }
    }
    return NULL;
}

const char *c2html(const char *src, const char *dst)
{
    FILE *src_fp = fopen(src, "rb");
    FILE *dst_fp = fopen(dst, "wb");
    if(src_fp == NULL || dst_fp == NULL) {
        if(src_fp != NULL) fclose(src_fp);
        if(dst_fp != NULL) fclose(dst_fp);
    }
    const char *err = c2html2(src_fp, dst_fp);
    fclose(src_fp);
    fclose(dst_fp);
    return err;
}