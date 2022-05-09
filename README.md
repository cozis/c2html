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
        1. [--template, --begin and --end](#--template---begin-and---end)
    1. [Using the library](#using-the-library)
1. [License](#license)

# Usage
c2html comes both as a C library and a command-line utility. 

## Using the command-line interface
You can highlight your C files by doing
```sh
c2html --input file.c --output file.html
```
which will read `file.c` and generate `file.html`. To know more, you can always run `c2html --help`. If the input and/or the output aren't provided, then `stdin` and `stdout` are used.

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

### --template, --begin and --end
When using the --template option, only the C code between the `<c2html>` and `</c2html>` tokens is processed. The remaining text is copied unchanged. This is useful when writing web pages containing C code. 

Lets say we have the following `index.html`
```html
<html>
  <head>
    <link rel="stylesheet" href="style.css" />
    <title>My awesome web page!</title>
  </head>
  <body>
    Hey, look at this C code:
    <c2html>
int main()
{
  print("Hello, world!\n");
  return 0;
}
    </c2html>
    Crazy, isn't it??
  </body>
</html>
```

the command 
```sh
c2html --input index.html --output processed_index.html --template
```
will output
```html
<html>
  <head>
    <link rel="stylesheet" href="style.css" />
    <title>My awesome web page!</title>
  </head>
  <body>
    Hey, look at this C code:
    <div class="c2h-code">
  <div class="c2h-code-inner">
    <table>
      <tr><td>1</td><td></td></tr>
      <tr><td>2</td><td><span class="c2h-kword c2h-kword-int">int</span> <span class="c2h-identifier c2h-fdeclname">main</span>()</td></tr>
      <tr><td>3</td><td>{</td></tr>
      <tr><td>4</td><td>  <span class="c2h-identifier c2h-fcallname">print</span>(<span class="c2h-val-str">"Hello, world!\n"</span>);</td></tr>
      <tr><td>5</td><td>  <span class="c2h-kword c2h-kword-return">return</span> <span class="c2h-val-int">0</span>;</td></tr>
      <tr><td>6</td><td>}</td></tr>
      <tr><td>7</td><td>    </td></tr>
    </table>
  </div>
</div>

    Crazy, isn't it??
  </body>
</html>
```

It's not possible to use `--style` in template mode.

It's possible to change the tokens that delimit the C code using the `--begin` and `--end` options. Lets say we want to change `<c2html>` and `</c2html>` them to `{start}` and `{end}`. We would do it by adding:

```sh
c2html --input index.html --output processed_index.html --template --begin "{start}" --end "{end}"
```

The `--begin` and `--end` can only be used alongside `--template`.

## Using the library
The library only exports one function
```c
char *c2html(const char *str, long len, const char *prefix,
             long *output_len, const char **error)
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

    char *html = c2html(c, -1, prefix, NULL, NULL);
    printf("%s\n", html);
    free(html);
    return 0;
}
```
when executed, the output will be:
```html
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
The code is very portable so it's possible to run it everywhere, although the build proces was only tested on Linux.

## Install the library
There is no particular way to install the library. The code is so small that you can just drop `c2html.c` and `c2html.h` in your project and use them as they were your own.

## Install the command-line interface
To build the CLI, run
```sh
make c2html
```
which will build the CLI executable `c2html`.

If you also want to install it, run
```sh
sudo make install
```
then you'll be able to use the `c2html` command in your terminal.

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