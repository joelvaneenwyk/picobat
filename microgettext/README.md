# `microgettext`

## A stupidly minimalist clone of GNU libintl

`microgettext` is a free minimalist clone of GNU libintl. Don't get me wrong, GNU gettext
in general has amazing features that make the work of maintaining NLS a breeze.
However, the `gettext` library (`libintl`) comes with a huge dependency wall (iconv, &c more
than 1MB of binaries on Windows).

`microgettext` is a single-file, extremely tiny implementation of libintl that enables the
use of gettext `*.mo` files without adding MBs to your binaries.

## How does it work ?

It is quite simple. Just download the `microgettext` archive and extract it in a folder
then in that folder just use `make`. This should generate a static library `microgettext.a`
that you can link against to use the gettext functions. Note that the include file for
microgettext is `gettext.h`

The functions provided by `microgettext` have merely the same behaviour as those provided
by GNU libintl. The only difference is that `microgettext` does not support plurals forms
yet and that it only does not takes the country part of the locale name into account
when searching .mo files (that is, `fr_FR` and `fr_CA` will all be routed to the `fr/`
folder in the locale tree).

`microgettext` is pure ANSI-C, so there is no need for any dependencies (you just need a
libC) thus `microgettext` is extremely portable.

## An example please ?

```C
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

#include "gettext.h"

int main(void)
{
 /* Set the locale to the system default (by default the
    locale of every C program is "C" */
 setlocale(LC_ALL, "");

 /* Select a base directory for "my-domain" *.mo files */
 bindtextdomain("my-domain", "/some/dir");

 /* Select a domain to use with gettext() by default */
 textdomain("my-domain");

 /* Print a well know, but translated message */
 printf("%s", gettext("Hello world !"));

 return 0;
}
```

## License

```plaintext
Copyright (c) 2021 Romain Garbi
```

`microgettext` is distributed under the MIT license. Use, distribution and modifications
are allowed without restrictions as long as the license statement is preserved.
