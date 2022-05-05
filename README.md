# c2html
A tool to add HTML syntax highlighting to C code.

# Installation and usage
c2html comes both as a C library and a command-line utility.

The library only exports one function
```c
char *c2html(const char *str, long len, _Bool table_mode, 
             const char *class_prefix, const char **error);
```
which, given a string of C code, returns the version highlighted using HTML tags.

For example, lets consider the 
```c
/* .. include stdlib.h, string.h and stdio.h .. */
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
```
will output:
```
<div class="code">
  <div class="code-inner">
    <span class="kword kword-int">int</span> <span class="identifier fdeclname">main</span>() {<br />
    &emsp;&emsp;&emsp;&emsp;<span class="kword kword-int">int</span> <span class="identifier">a</span> <span class="operator">=</span> <span class="val-int">5</span>;<br />
    &emsp;&emsp;&emsp;&emsp;<span class="kword kword-return">return</span> <span class="val-int">0</span>;<br />
    }<br />
  </div>
</div>
```
if `table_mode` were `1`, then the output would have been:
```
<div class="code">
  <div class="code-inner">
    <table>
      <tr><td>1</td><td><span class="kword kword-int">int</span> <span class="identifier fdeclname">main</span>() {</td></tr>
      <tr><td>2</td><td>&emsp;&emsp;<span class="kword kword-int">int</span> <span class="identifier">a</span> <span class="operator">=</span> <span class="val-int">5</span>;</td></tr>
      <tr><td>3</td><td>&emsp;&emsp;<span class="kword kword-return">return</span> <span class="val-int">0</span>;</td></tr>
      <tr><td>4</td><td>}</td></tr>
      <tr><td>5</td><td></td></tr>
    </table>
  </div>
</div>
```
the color doesn't come with the generated HTML, you need to add it yourself using CSS. There's an example CSS style provided in `style.css`, inspired by the Sublime Text color theme I use.

Since the generated class names of the HTML tags are pretty generic (`identifier`, `operator`, `comment`, ..), you can specify a prefix to be prepented to these names. 