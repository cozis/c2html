#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

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
} Kind;

typedef struct { 
    Kind kind; int off, len; 
} Token;

static bool isoperat(char c)
{
    return c == '+' 
        || c == '-'
        || c == '*'
        || c == '/'
        || c == '%'
        || c == '='
        || c == '!'
        || c == '<'
        || c == '>'
        || c == '|'
        || c == '&';
}

static bool iskword(const char *str, long len)
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
        if(strlen(keywords[i]) == len && !strncmp(keywords[i], str, len))
            return 1;
    return 0;
}

static Token *tokenize(const char *str, long len)
{
    Token *array = NULL;
    int count = 0, capacity = 0;

    long i = 0;

    Token T;
    long curly_bracket_depth = 0;
    long bracket_depth = 0;
    do {
        if(i == len) {
            T.kind = T_DONE;
            T.off = i;
            T.len = 0;
        } else if(i+1 < len && str[i] == '/' && str[i+1] == '/') {
            T.kind = T_COMMENT;
            T.off = i;
            while(i < len && str[i] != '\n')
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
            while(i < len && isdigit(str[i]))
                i += 1;
            if(i+1 < len && str[i] == '.' && isdigit(str[i+1])) {
                i += 1; // Skip the '.'.
                while(i < len && isdigit(str[i]))
                    i += 1;
                T.kind = T_VFLT;
            } else T.kind = T_VINT;
            T.len = i - T.off;
        } else if(isoperat(str[i])) {
            T.kind = T_OPERATOR;
            T.off = i;
            while(i < len && isoperat(str[i]))
                i += 1;
            T.len = i - T.off;
        } else if(isalpha(str[i]) || str[i] == '_') {
            T.off = i;
            while(isalpha(str[i]) || isdigit(str[i]) || str[i] == '_')
                i += 1;
            T.len = i - T.off;
            if(iskword(str + T.off, T.len))
                T.kind = T_KWORD;
            else {
                // Is the identifier followed by 
                // a left parenthesis?
                long k = i;
                while(k < len && (str[k] == ' ' || str[k] == '\t'))
                    k += 1;
                if(k < len && str[k] == '(') {
                    if(curly_bracket_depth == 0)
                        T.kind = T_FDECLNAME;
                    else
                        T.kind = T_FCALLNAME;
                } else {
                    T.kind = T_IDENTIFIER;
                }
            }
        } else {

            switch(str[i]) {
                case '{': curly_bracket_depth += 1; break;
                case '}': curly_bracket_depth -= 1; break;
                case '(': bracket_depth += 1; break;
                case ')': bracket_depth -= 1; break;
            }

            T.kind = str[i];
            T.off = i;
            T.len = 1;
            i += 1;
        }

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

        assert(str[j] == '<' || str[j] == '>');
        if(str[j] == '<')
            buff_printf(buff, "&lt;");
        else
            buff_printf(buff, "&gt;");
        j += 1;
    }
}

char *c2html(const char *str, long len, _Bool table_mode, const char *class_prefix, const char **error) {

    if(str == NULL)
        str = "";

    if(len < 0)
        len = strlen(str);

    if(class_prefix == NULL)
        class_prefix = "";

    buff_t buff;
    buff_init(&buff);
    long lineno = 1;

    buff_printf(&buff, "\n"
        "<div class=\"%scode\">\n"
        "  <div class=\"%scode-inner\">\n",
        class_prefix, class_prefix);

    if(table_mode)
        buff_printf(&buff, "    <table>\n"
                           "      <tr><td>1</td><td>");

    Token *tokens = tokenize(str, len);
    for(int i = 0; tokens[i].kind != T_DONE; i += 1) {

        switch(tokens[i].kind) {

            case T_DONE:
            assert(0);
            break;

            case T_NEWL:
            if(table_mode)
                for(int j = 0; j < tokens[i].len; j += 1) {
                    lineno += 1;
                    buff_printf(&buff, "</td></tr>\n      <tr><td>%d</td><td>", lineno);
                }
            else {
                lineno += tokens[i].len;
                for(int j = 0; j < tokens[i].len; j += 1)
                    buff_printf(&buff, "<br />\n");
            }
            break;

            case T_SPACE:
            if(tokens[i].len == 1)
                buff_printf(&buff, " ", 1);
            else {
                for(int j = 0; j < tokens[i].len; j += 1)
                    buff_printf(&buff, "&emsp;");
            }
            break;

            case T_TAB:
            for(int j = 0; j < tokens[i].len; j += 1)
                buff_printf(&buff, "&emsp;&emsp;&emsp;&emsp;");
            break;

            case T_KWORD:
            buff_printf(&buff, "<span class=\"%skword %skword-%.*s\">%.*s</span>",
                class_prefix, class_prefix,
                tokens[i].len, str + tokens[i].off,
                tokens[i].len, str + tokens[i].off);
            break;

            case T_VSTR:
            buff_printf(&buff, "<span class=\"%sval-str\">", class_prefix);
            print_escaped(&buff, str + tokens[i].off, tokens[i].len);
            buff_printf(&buff, "</span>");
            break;

            case T_VCHAR:
            buff_printf(&buff, "<span class=\"%sval-char\">", class_prefix);
            print_escaped(&buff, str + tokens[i].off, tokens[i].len);
            buff_printf(&buff, "</span>");
            break;

            case T_VINT:
            buff_printf(&buff, "<span class=\"%sval-int\">%.*s</span>",
                class_prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_VFLT:
            buff_printf(&buff, "<span class=\"%sval-flt\">%.*s</span>",
                class_prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_FDECLNAME:
            buff_printf(&buff, "<span class=\"%sidentifier %sfdeclname\">%.*s</span>",
                class_prefix, class_prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_FCALLNAME:
            buff_printf(&buff, "<span class=\"%sidentifier %sfcallname\">%.*s</span>",
                class_prefix, class_prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_IDENTIFIER:
            buff_printf(&buff, "<span class=\"%sidentifier\">%.*s</span>",
                class_prefix, tokens[i].len, str + tokens[i].off);
            break;

            case T_COMMENT:
            buff_printf(&buff, "<span class=\"%scomment\">", class_prefix);
            print_escaped(&buff, str + tokens[i].off, tokens[i].len);
            buff_printf(&buff, "</span>");
            break;

            case T_OPERATOR:
            buff_printf(&buff, "<span class=\"%soperator\">", class_prefix);
            print_escaped(&buff, str + tokens[i].off, tokens[i].len);
            buff_printf(&buff, "</span>");
            break;

            default:
            buff_printf(&buff, "%c", str[tokens[i].off]);
            break;
        }
    }

    if(table_mode)
        buff_printf(&buff, "</td></tr>\n    </table>\n");
    buff_printf(&buff, "  </div>\n"
                       "</div>");

    char *res;
    if(buff.error == NULL) {
        buff.data[buff.used] = '\0';
        res = buff.data;
    } else {
        if(error != NULL)
            *error = buff.error;
        res = NULL;
    }

    free(tokens);
    return res;
}