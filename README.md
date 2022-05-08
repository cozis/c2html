# c2html
A tool to add HTML syntax highlighting to C code.

Basically you give `c2html` some C code, and it annotates it with HTML `<span>` elements that have class names describing the type of token. 

For example, by providing it with the code
```c
int a;
```
the output is
```html
<div class="c2h-code">
  <div class="c2h-code-inner">
    <table>
      <tr>
        <td>1</td>
        <td>
          <span class="c2h-kword c2h-kword-int">int</span> <span class="c2h-identifier">a</span>;
        </td>
      </tr>
    </table>
  </div>
</div>
```
therefore you can apply custom color schemes by selecting the tokens from your CSS. 

A default stylesheet you can use is provided in `style.css`. It's also well documented, so that you can go off and write your own!

# Index
1. [Install](#install)
    1. [Supported platforms](#supported-platforms)
    1. [Install the library](#install-the-library)
    1. [Install the command-line interface](#install-the-command-line-interface)
1. [Usage](#usage)
    1. [Using the command-line interface](#using-the-command-line-interface)
        1. [--style](#--style)
        1. [--prefix](#--prefix)
    1. [Using the library](#using-the-library)
1. [License](#license)

# Usage
c2html comes both as a C library and a command-line utility. 

## Using the command-line interface
You can highlight your C files by doing
```sh
c2html --input file.c --output file.html
```
which will read `file.c` and generate `file.html`. To know more, you can always run `c2html --help`.

### --style
The HTML comes with no styling. If you want to apply a CSS to it, you can provide to `c2html` a style file using the `--style` option followed by the name of the file.

```sh
c2html --input file.c --output file.html --style style.css
```

This will basically add a `<style>` element with the contents of the `style.css` file before the normal HTML output.

### --prefix
By default, all of the HTML class names are prefixed with `c2h-` to avoid namespace collisions with your code. You can change the prefix using the `--prefix` option, like this:
```sh
c2html --input file.c --output file.html --prefix myprefix-
```
in which case, identifiers will be generated with the `myprefix-identifier` class name instead of the usual `c2h-identifier`.

## Using the library
The library only exports one function
```c
char *c2html(const char *str, long len, 
             const char *prefix, const char **error);

```
Given a string containing C code, returns the highlighted version using HTML `<span>` tags. (You can find a complete description of what it does in `c2html.h`)

For example, consider the following C code:
```c
/* .. include stdlib.h, string.h and stdio.h .. */
#include "c2html.h"

int main()
{
    const char *prefix = NULL;

    char *c = 
      "int main() {\n"
      "  int a = 5;\n"
      "  return 0;\n"
      "}\n";

    char *html = c2html(c, strlen(c), prefix, NULL);
    printf("%s\n", html);
    free(html);
    return 0;
}
```
when executed, the output will be:
```
<div class="code">
  <div class="code-inner">
    <table>
      <tr><td>1</td><td><span class="kword kword-int">int</span> <span class="identifier fdeclname">main</span>() {</td></tr>
      <tr><td>2</td><td>  <span class="kword kword-int">int</span> <span class="identifier">a</span> <span class="operator">=</span> <span class="val-int">5</span>;</td></tr>
      <tr><td>3</td><td>  <span class="kword kword-return">return</span> <span class="val-int">0</span>;</td></tr>
      <tr><td>4</td><td>}</td></tr>
      <tr><td>5</td><td></td></tr>
    </table>
  </div>
</div>

```

# Install

## Supported platforms
The code is very portable so it's possible to run it everywhere, although there are only build and install scripts for \*nix systems.

## Install the library
There is no particular way to install the library. The code is so small that you can just drop `c2html.c` and `c2html.h` in your project and use then as they were your own.

## Install the command-line interface
To install the `c2html` command under **linux**, you first have to build it by running `build.sh`, then you can install it with `install.sh`.

You may need to give these scripts execution privileges first. You can do that by running `chmod +x build.sh` and `chmod +x install.sh`.

Once the CLI is installed, you'll be able to use the `c2html` command in your terminal.

# License
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>