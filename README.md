# c2html
A tool to add HTML syntax highlighting to C code.

Basicaly you give `c2html` some C code as input and it classifies all the keywords, identifiers etc using `<span>` elements, associating them with the appropriate class names. By applying the `style.css` stylesheet to the generated output, you get the highliting. If you prefer, you can write your own style.

# Index
1. [Install](#install)
  1. [Supported platforms](#supported-platforms)
  2. [Install the library](#install-the-library)
  3. [Install the command-line interface](#install-the-command-line-interface)
2. [Usage](#usage)
  1. [Using the command-line interface](#using-the-command-line-interface)
    1. [--no-table](#--no-table)
    2. [--style](#--style)
  2. [Using the library](#using-the-library)

# Install

## Supported platforms
The code is very portable so it's be possible to run it everywhere, although there are only a build and install script for \*nix systems.

## Intall the library
To install the library, you just need to copy the `c2html.c` and `c2html.h` files wherever you want to use them and compile them as they were your files. Since the library is so small, you can also just copy the contents of `c2html` in your own project.

## Install the command-line interface
To install the `c2html` command under **linux**, you first have to build it by running `build.sh`, then you can install it with `install.sh`.

You may need to give these scripts execution privileges first. You can do that by running `chmod +x build.sh` and `chmod +x install.sh`.

# Usage
c2html comes both as a C library and a command-line utility. 

## Using the command-line interface
By running `build.sh`, the `c2html` executable is built, which is command-line interface of c2html.

You can highlight your C files by doing
```sh
./c2html --input file.c --output file.html
```
This command will generate the highlighted C code.

### --no-table
Normally, `c2html` will generate html using a `<table>` element, where each line is a `<tr>` element. This makes the output kind of big. By using the `--no-table` option, it's possible to generate a more lightweight output where lines are splitted using `<br/>` elements instead of using a `<table>`.

You'd use it like this;
```sh
./c2html --input file.c --output file.html --no-table
```

### --style
The HTML comes with no styling. If you want to apply a CSS to it, you can provide to `c2html` a style file using the `--style` option followed by the name of the file.

```sh
./c2html --input file.c --output file.html --style style.css
```

This will basically add a `<style>` element with the contents of the `style.css` file before the normal HTML output.

## Using the library
The library only exports one function
```c
char *c2html(const char *str, long len, _Bool table_mode, 
             const char *class_prefix, const char **error);
```
which, given a string containing C code, returns the highlighted version using HTML tags.

For example, lets consider the 
```c
/* .. include stdlib.h, string.h and stdio.h .. */
#include "c2html.h"

int main()
{
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