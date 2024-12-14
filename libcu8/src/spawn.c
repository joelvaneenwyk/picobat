/*

 libcu8 - A wrapper to fix msvcrt utf8 incompatibilities issues
 Copyright (c) 2014, 2015, 2016 Romain GARBI

 All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of the name of Romain Garbi nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY ROMAIN GARBI AND CONTRIBUTORS ``AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL ROMAIN GARBI AND CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES
 LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#if defined(WIN32)
#include <io.h>
#include <windows.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

#include "iconv/iconv.h"

#include "internals.h"

#include <libcu8.h>

#if !defined(__linux__)
extern char **_environ;
extern wchar_t **_wenviron;
#define ENVIRON_PTR (const char *const *)_environ
#define WENVIRON_PTR (const wchar_t * const*)_wenviron
#else
extern char **__environ;
#define ENVIRON_PTR (const char *const *)__environ
#define WENVIRON_PTR (const char *const *)__environ
#endif

__LIBCU8__IMP __cdecl intptr_t libcu8_spawnl(int mode, const char* file, ...)
{
    va_list args;
    const char* argv[1024];
    int i=0;
    intptr_t ret;

    va_start(args, file);

    while ( (i < 1024) && (argv[1] = va_arg(args, const char*)))
        i++;

    va_end(args);

    if (i == 1024)
        return -1;

    return libcu8_spawnve(mode, file, (const char * const*)args, ENVIRON_PTR);
}

__LIBCU8__IMP __cdecl intptr_t libcu8_spawnle(int mode, const char* file, ...)
{
    va_list args;
    const char* argv[1024];
    const char* const* env;
    int i=0;
    intptr_t ret;

    va_start(args, file);

    while ( (i < 1024) && (argv[1] = va_arg(args, const char*)))
        i++;

    va_end(args);

    if (i == 1024 || i <= 1)
        return -1;

    env = (const char * const*)argv[i-2];
    argv[i-2] = NULL;
    return libcu8_spawnve(mode, file, (const char * const*)args, env);
}

__LIBCU8__IMP __cdecl intptr_t libcu8_spawnlp(int mode, const char* file, ...)
{
    va_list args;
    const char* argv[1024];
    int i=0;
    intptr_t ret;

    va_start(args, file);

    while ( (i < 1024) && (argv[1] = va_arg(args, const char*)))
        i++;

    va_end(args);

    if (i == 1024)
        return -1;

    return libcu8_spawnvpe(mode, file, (const char * const*)args, ENVIRON_PTR);
}

__LIBCU8__IMP __cdecl intptr_t libcu8_spawnlpe(int mode, const char* file, ...)
{
    va_list args;
    const char* argv[1024];
    const char* const* env;
    int i=0;
    intptr_t ret;

    va_start(args, file);

    while ( (i < 1024) && (argv[1] = va_arg(args, const char*)))
        i++;

    va_end(args);

    if (i == 1024 || i <= 1)
        return -1;

    env = (const char * const*)argv[i-2];
    argv[i-2] = NULL;
    return libcu8_spawnvpe(mode, file, (const char * const*)args, (const char * const*)env);
}

__LIBCU8__IMP __cdecl intptr_t libcu8_spawnv(int mode, const char* file,
                                                    const char* const *argv)
{
    return (intptr_t)libcu8_spawnve(mode, file, argv, ENVIRON_PTR);
}

__LIBCU8__IMP __cdecl intptr_t libcu8_spawnve(int mode, const char* file,
                            const char* const *argv, const char *const *envp)
{
    wchar_t *wfile, **wargv;
    size_t cvt;
    int i=0, j=0, ret;

    /* count how long the wargv should be */
    while (argv[i] != NULL)
        i++;

    if (!(wargv = malloc((i+1)*sizeof(wchar_t*))) ||
        !(wfile = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, file, strlen(file)+1, &cvt)))
        goto error;

    for (j; j < i ; j++) {

        if ((wargv[j] = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, argv[j],
                                        strlen(argv[j]) + 1, &cvt)) == NULL)
            goto error;

    }

    wargv[j] = NULL;

    ret = _wspawnve(mode, wfile, (const wchar_t* const *)wargv, WENVIRON_PTR);

    free(wfile);

    for (i=0; i < j; i++)
        free(wargv[i]);

    free(wargv);

    return ret;

error :

    if (wfile)
        free(wfile);

    if (j)
        for (i = 0; i < j; j++)
            free((char*)argv[i]);

    free(wargv);

    return -1;
}

__LIBCU8__IMP __cdecl intptr_t libcu8_spawnvp(int mode, const char* file,
                                                const char* const *argv)
{
    return libcu8_spawnvpe(mode, file, argv, ENVIRON_PTR);
}

__LIBCU8__IMP __cdecl intptr_t libcu8_spawnvpe(int mode, const char* file,
                                                    const char* const *argv,
                                                    const char *const *envp)
{
    wchar_t *wfile, **wargv;
    size_t cvt;
    int i=0, j=0, ret;

    /* count how long the wargv should be */
    while (argv[i] != NULL)
        i++;

    if (!(wargv = malloc((i+1)*sizeof(wchar_t*))) ||
        !(wfile = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, file, strlen(file)+1, &cvt)))
        goto error;

    for (j; j < i ; j++) {

        if ((wargv[j] = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, argv[j],
                                        strlen(argv[j]) + 1, &cvt)) == NULL)
            goto error;

    }

    wargv[j] = NULL;

#if defined(WIN32)
    ret = _wspawnvpe(mode, wfile, (const wchar_t * const*)wargv, (const wchar_t * const*)_wenviron);
#else
    ret = execvpe(file, (char * const*)argv, envp);
#endif

    free(wfile);

    for (i=0; i < j; i++)
        free((char*)wargv[i]);

    free(wargv);

    return ret;

error :

    if (wfile)
        free(wfile);

    if (j)
        for (i = 0; i < j; j++)
            free((char*)argv[i]);


    return -1;
}
