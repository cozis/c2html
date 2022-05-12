/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  *
 * +------------------------------------------------------------+ *
 * | This is free  and unencumbered software  released into the | *
 * | public domain.                                             | *
 * |                                                            | *
 * | Anyone  is  free to copy, modify, publish,  use,  compile, | *
 * | sell,  or  distribute this software, either in source code | *
 * | form or as a compiled binary, for any  purpose, commercial | * 
 * | or non-commercial, and by any means.                       | *
 * |                                                            | *
 * | In jurisdictions that recognize copyright laws, the author | *
 * | or authors of this software dedicate any and all copyright | *
 * | interest  in  the software  to  the public domain. We make | *
 * | this dedication for the benefit of the public at large and | *
 * | to the detriment  of  our heirs and successors.  We intend | *
 * | this dedication to be an  overt  act of  relinquishment in | *
 * | perpetuity  of  all  present  and  future  rights  to this | *
 * | software under copyright law.                              | *
 * |                                                            | *
 * | THE  SOFTWARE IS PROVIDED  "AS IS",  WITHOUT  WARRANTY  OF | *
 * | ANY KIND,  EXPRESS OR IMPLIED, INCLUDING BUT  NOT  LIMITED | *
 * | TO  THE  WARRANTIES  OF  MERCHANTABILITY,  FITNESS  FOR  A | *
 * | PARTICULAR PURPOSE AND NONINFRINGEMENT. IN  NO EVENT SHALL | *
 * | THE  AUTHORS  BE LIABLE FOR ANY CLAIM, DAMAGES  OR  OTHER  | *
 * | LIABILITY, WHETHER  IN  AN  ACTION  OF  CONTRACT, TORT  OR | *
 * | OTHERWISE, ARISING  FROM, OUT  OF  OR  IN  CONNECTION WITH | *
 * | THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. | *  *
 * +------------------------------------------------------------+--+ *
 * | For more information, please refer to <http://unlicense.org/> | *
 * +---------------------------------------------------------------+ *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include "c2html.h"

typedef enum {
    T_DONE = 256,
    T_COMMENT,
    T_SPACE,
    T_TAB,
    T_NEWL,
    T_VSTR,
    T_VCHAR,
    T_VINT,
    T_VFLT,
    T_KWORD,
    T_FDECLNAME,
    T_FCALLNAME,
    T_IDENTIFIER,
    T_OPERATOR,
    T_DIRECTIVE,
} Kind;

typedef struct { 
    Kind kind; int off, len; 
} Token;

static bool isoperat(char c)
{
    return c == '+' || c == '-'
        || c == '*' || c == '/'
        || c == '%' || c == '='
        || c == '!'
        || c == '<' || c == '>'
        || c == '|' || c == '&';
}

static bool iskword(const char *str, long len)
{
    static const struct {
        int  len;
        char str[8]; // Maximum length of a keyword.
    } keywords[] = {
        #define KWORD(lit) { sizeof(lit)-1, lit }
        KWORD("auto"),     KWORD("break"),   KWORD("case"), 
        KWORD("char"),     KWORD("const"),   KWORD("continue"), 
        KWORD("default"),  KWORD("do"),      KWORD("double"), 
        KWORD("else"),     KWORD("enum"),    KWORD("extern"), 
        KWORD("float"),    KWORD("for"),     KWORD("goto"),
        KWORD("if"),       KWORD("int"),     KWORD("long"), 
        KWORD("register"), KWORD("return"),  KWORD("short"), 
        KWORD("signed"),   KWORD("sizeof"),  KWORD("static"), 
        KWORD("struct"),   KWORD("switch"),  KWORD("typedef"), 
        KWORD("union"),    KWORD("unsigned"),KWORD("void"), 
        KWORD("volatile"), KWORD("while"),
        #undef KWORD
    };

    const int num_keywords = sizeof(keywords)/sizeof(keywords[0]);
    for(int i = 0; i < num_keywords; i += 1)
        if(keywords[i].len == len && !strncmp(keywords[i].str, str, len))
            return true;
    return false;
}

static Token *tokenize(const char *str, long len)
{
    Token *array = NULL;
    int count = 0, capacity = 0;

    long i = 0;

    Token T;
    long curly_bracket_depth = 0;
    bool only_spaces_since_line_start = true;
    bool prev_nonspace_was_directive = false;

    do {
        if(i == len) {
            T.kind = T_DONE;
            T.off = i;
            T.len = 0;
        } else if(i+1 < len && str[i] == '/' && str[i+1] == '/') {
            T.kind = T_COMMENT;
            T.off = i;
            while(i < len && str[i] != '\n') // What about backslashes??
                i += 1;
            T.len = i - T.off;
        } else if(i+1 < len && str[i] == '/' && str[i+1] == '*') {
            T.kind = T_COMMENT;
            T.off = i;
            while(1) {

                while(i < len && str[i] != '*')
                    i += 1;

                if(i == len)
                    break;

                assert(str[i] == '*');
                i += 1;

                if(i == len)
                    break;

                if(str[i] == '/') {
                    i += 1;
                    break;
                }
            }
            T.len = i - T.off;
        } else if(str[i] == ' ') {
            T.kind = T_SPACE;
            T.off = i;
            do
                i += 1;
            while(i < len && str[i] == ' ');
            T.len = i - T.off;
        } else if(str[i] == '\t') {
            T.kind = T_TAB;
            T.off = i;
            do
                i += 1;
            while(i < len && str[i] == ' ');
            T.len = i - T.off;
        } else if(str[i] == '\n') {
            T.kind = T_NEWL;
            T.off = i;
            do
                i += 1;
            while(i < len && str[i] == '\n');
            T.len = i - T.off;
        } else if(str[i] == '\'' || str[i] == '\"') {
            
            char f = str[i];

            T.kind = f == '"' ? T_VSTR : T_VCHAR;
            T.off = i;

            i += 1; // Skip the '\'' or '"'.
            
            do {
                while(i < len && str[i] != f && str[i] != '\\')
                    i += 1;

                if(i == len || str[i] == f)
                    break;

                if(str[i] == '\\') {
                    i += 1; // Skip the '\\'.
                    if(i < len)
                        i += 1; // ..and the character after it.
                }

            } while(1);

            if(i < len) {
                assert(str[i] == f);
                i += 1; // Skip the final '\'' or '"'.
            }
            T.len = i - T.off;

        } else if(isdigit(str[i])) {

            T.off = i;

            // We allow an 'x' if it's after a '0'.
            if(i+2 < len && str[i] == '0' && str[i+1] == 'x' && isdigit(str[i+2]))
                i += 2; // Skip the '0x'.

            while(i < len && isdigit(str[i]))
                i += 1;
            
            // If the next character is a dot followed
            // by a digit, then we continue to scan.    
            if(i+1 < len && str[i] == '.' && isdigit(str[i+1])) {
                i += 1; // Skip the '.'.
                while(i < len && isdigit(str[i]))
                    i += 1;
                T.kind = T_VFLT;
            } else T.kind = T_VINT;
            
            T.len = i - T.off;
        
        } else if(isalpha(str[i]) || str[i] == '_') {

            T.off = i;
            do
                i += 1;
            while(isalpha(str[i]) || 
                  isdigit(str[i]) || str[i] == '_');
            T.len = i - T.off;

            /* It may either be an identifier or a
             * language keyword.
             */
            
            if(iskword(str + T.off, T.len))
                T.kind = T_KWORD;
            else {

                /* If the identifier is followed by a '(' and
                 * it's in the global scope, then it's for a
                 * function definiton. If it's not in the global
                 * scope then it's a function call.
                 * Between the identifier and the '(' there may
                 * be some whitespace. An exception is made if
                 * before the identifier comes a preprocessor
                 * directive, in which case the '(' must come
                 * right after the identifier.
                 */

                bool followed_by_parenthesis = false;
                bool yes_and_immediately = false;
                {
                    long k = i;
                    while(k < len && (str[k] == ' ' || str[k] == '\t'))
                        k += 1;

                    if(k < len && str[k] == '(') {
                        followed_by_parenthesis = true;
                        if(k == i)
                            yes_and_immediately = true;
                    }
                }

                if(followed_by_parenthesis) {
                    if(curly_bracket_depth == 0) {
                        if(prev_nonspace_was_directive) {
                            if(yes_and_immediately)
                                T.kind = T_FDECLNAME;
                            else
                                T.kind = T_IDENTIFIER;
                        } else
                            T.kind = T_FDECLNAME;
                    } else
                        T.kind = T_FCALLNAME;
                } else {
                    T.kind = T_IDENTIFIER;
                }
            }
        
        } else if(str[i] == '#' && only_spaces_since_line_start) {

            // The first non-whitespace token of the line
            // is a '#'. If it's followed by an alphabetical
            // character, then it's a directive. (There may
            // be whitespace between the '#' and the identifier)

            long j = i; // Use a secondary cursor to explore
                        // what's after the '#'.

            j += 1; // Skip the '#'.

            // Skip spaces after the '#', if there are any.
            while(j < len && (str[j] == ' ' || str[j] == '\t'))
                j += 1;

            if(isalpha(str[j])) {

                // It's a preprocessor directive!
                
                T.kind = T_DIRECTIVE;
                T.off = i;
                
                while(j < len && isalpha(str[j]))
                    j += 1;

                T.len = j - T.off;
                
                i = j;

            } else {
                // Wasn't a directive.. Just tokenize the '#'.
                T.kind = '#';
                T.off = i;
                T.len = 1;
                i += 1;
            }

        } else if(str[i] == '<' && prev_nonspace_was_directive) {

            T.kind = T_VSTR;
            T.off = i;
            while(i < len && str[i] != '>')
                i += 1;
            if(i < len)
                i += 1; // Skip the '>'.
            T.len = i - T.off;

        } else if(isoperat(str[i])) {
            T.kind = T_OPERATOR;
            T.off = i;
            while(i < len && isoperat(str[i]))
                i += 1;
            T.len = i - T.off;
        } else {

            switch(str[i]) {
                case '{': curly_bracket_depth += 1; break;
                case '}': curly_bracket_depth -= 1; break;
            }

            T.kind = str[i];
            T.off = i;
            T.len = 1;
            i += 1;
        }

        if(T.kind == T_NEWL)
            only_spaces_since_line_start = true;
        else
            if(T.kind != T_TAB && T.kind != T_SPACE)
                only_spaces_since_line_start = false;

        if(T.kind == T_DIRECTIVE)
            prev_nonspace_was_directive = true;
        else
            if(T.kind != T_TAB && T.kind != T_SPACE)
                prev_nonspace_was_directive = false;

        if(count == capacity) {
            int new_capacity;
            if(capacity == 0)
                new_capacity = 8;
            else
                new_capacity = 2 * capacity;
        
            void *temp = realloc(array, new_capacity * sizeof(Token));
            if(temp == NULL) {
                // Uoops! Out of memory.
                free(array);
                return NULL;
            }

            array = temp;
            capacity = new_capacity;
        }

        array[count++] = T;

    } while(T.kind != T_DONE);
    return array;
}

typedef struct {
    char *error;
    char  *data;
    long   size;
    long   used;
} buff_t;

static void buff_init(buff_t *buff)
{
    memset(buff, 0, sizeof(buff_t));
}

static void buff_puts(buff_t *buff, const char *str, long len) {

    if(buff->error)
        return;

    if(buff->used + len > buff->size) {

        int new_size;
        if(buff->size == 0)
            new_size = 32;
        else
            new_size = 2 * buff->size;

        if(buff->used + len > new_size)
            new_size = buff->used + len;

        void *temp = realloc(buff->data, new_size+1);
        if(temp == NULL) {
            free(buff->data);
            buff->error = "Out of memory";
            return;
        }

        buff->data = temp;
        buff->size = new_size;
    }

    memcpy(buff->data + buff->used, str, len);
    buff->used += len;
}

static void buff_printf(buff_t *buff, const char *fmt, ...)
{
    char maybe[512];
    char *buffer;
    va_list va, va2;
    va_start(va, fmt);
    va_copy(va2, va);

    int n = vsnprintf(maybe, sizeof(maybe), fmt, va);
    if(n < 0) {
        free(buff->data);
        buff->error = "Bad format";
        return;
    }

    if(n < (int) sizeof(maybe))
        buffer = maybe;
    else {
        
        buffer = malloc(n+1);
        if(buffer == NULL) {
            free(buff->data);
            buff->error = "Out of memory";
            goto done;
        }

        int k = vsnprintf(buffer, n+1, fmt, va2);
        assert(k >= 0 && k == n);
        (void) k; // If asserts are deactivated by defining
                  // NDEBUG, the compiler complains because
                  // k is unused.
    }

    assert(buffer[n] == '\0');
    buff_puts(buff, buffer, n);

done:
    if(buffer != maybe)
        free(buffer);
    va_end(va2);
    va_end(va);
}

static void print_escaped(buff_t *buff, const char *str, long len)
{
    long j = 0;
    while(1) {

        long off = j;

        while(j < len && str[j] != '<' && str[j] != '>')
            j += 1;

        long end = j;
        buff_puts(buff, str + off, end - off);

        if(j == len)
            break;

        switch(str[j]) {
            case '<': buff_printf(buff, "&lt;"); break;
            case '>': buff_printf(buff, "&gt;"); break;
            default: assert(0); break;
        }

        j += 1;
    }
}

char *c2html(const char *str, long len, const char *prefix, 
             long *output_len, const char **error)
{
    if(str == NULL)
        str = "";

    if(len < 0)
        len = strlen(str);

    if(prefix == NULL)
        prefix = "";

    Token *tokens = tokenize(str, len);
    if(tokens == NULL) {
        if(error != NULL)
            *error = "Out of memory";
        return NULL;
    }

    buff_t buff;
    buff_init(&buff);
    long lineno = 1;

    buff_printf(&buff,
        "<div class=\"%scode\">\n"
        "  <div class=\"%scode-inner\">\n"
        "    <table>\n"
        "      <tr><td>1</td><td>",
        prefix, prefix);

    for(int i = 0; tokens[i].kind != T_DONE; i += 1) {

        switch(tokens[i].kind) {

            case T_DONE:
            assert(0);
            break;

            case T_NEWL:
            for(int j = 0; j < tokens[i].len; j += 1) {
                lineno += 1;
                buff_printf(&buff, "</td></tr>\n      <tr><td>%d</td><td>", lineno);
            }
            break;

            case T_SPACE:
            buff_puts(&buff, str + tokens[i].off, tokens[i].len);
            break;

            case T_TAB:
            for(int j = 0; j < tokens[i].len; j += 1)
                buff_printf(&buff, "    ");
            break;

            case T_KWORD:
            buff_printf(&buff, "<span class=\"%skword %skword-%.*s\">%.*s</span>",
                prefix, prefix,
                tokens[i].len, str + tokens[i].off,
                tokens[i].len, str + tokens[i].off);
            break;

            case T_VSTR:
            buff_printf(&buff, "<span class=\"%sval-str\">", prefix);
            print_escaped(&buff, str + tokens[i].off, tokens[i].len);
            buff_printf(&buff, "</span>");
            break;

            case T_VCHAR:
            buff_printf(&buff, "<span class=\"%sval-char\">", prefix);
            print_escaped(&buff, str + tokens[i].off, tokens[i].len);
            buff_printf(&buff, "</span>");
            break;

            case T_VINT:
            buff_printf(&buff, "<span class=\"%sval-int\">%.*s</span>",
                prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_VFLT:
            buff_printf(&buff, "<span class=\"%sval-flt\">%.*s</span>",
                prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_FDECLNAME:
            buff_printf(&buff, "<span class=\"%sidentifier %sfdeclname\">%.*s</span>",
                prefix, prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_FCALLNAME:
            buff_printf(&buff, "<span class=\"%sidentifier %sfcallname\">%.*s</span>",
                prefix, prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_IDENTIFIER:
            buff_printf(&buff, "<span class=\"%sidentifier\">%.*s</span>",
                prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_COMMENT:
            {
                long j = tokens[i].off;
                long end = j + tokens[i].len;
                while(1) {

                    long line_off = j;
                    while(j < end && str[j] != '\n')
                        j += 1;
                    long line_len = j - line_off;

                    buff_printf(&buff, "<span class=\"%scomment\">", prefix);
                    print_escaped(&buff, str + line_off, line_len);
                    buff_printf(&buff, "</span>");

                    if(j == end)
                        break;

                    j += 1; // Skip the '\n'.

                    lineno += 1;
                    buff_printf(&buff, "</td></tr>\n      <tr><td>%d</td><td>", lineno);
                }
                break;
            }

            case T_OPERATOR:
            buff_printf(&buff, "<span class=\"%soperator\">", prefix);
            print_escaped(&buff, str + tokens[i].off, tokens[i].len);
            buff_printf(&buff, "</span>");
            break;

            case T_DIRECTIVE:
            buff_printf(&buff, "<span class=\"%sdirective\">", prefix);
            print_escaped(&buff, str + tokens[i].off, tokens[i].len);
            buff_printf(&buff, "</span>");
            break;

            default:
            buff_printf(&buff, "%c", str[tokens[i].off]);
            break;
        }
    }

    buff_printf(&buff, 
                  "</td></tr>\n"
            "    </table>\n"
            "  </div>\n"
            "</div>\n");

    char *res;
    if(buff.error == NULL) {
        buff.data[buff.used] = '\0';
        res = buff.data;
    } else {
        if(error != NULL)
            *error = buff.error;
        res = NULL;
    }

    if(output_len != NULL)
        *output_len = buff.used;

    free(tokens);
    return res;
}