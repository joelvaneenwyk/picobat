/*

 libcu8 - A wrapper to fix msvcrt utf8 incompatibilities issues
 Copyright (c) 2014, 2015, 2017 Romain GARBI

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
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <process.h>
#include <direct.h>
#else
#include <unistd.h>
#include <stdint.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "iconv/iconv.h"

#include "internals.h"
#include "config.h"

#if 0
#define REPLACE_FN( )

struct fn_replace_t functions[] = {
    {"_read", libcu8_read},
    {"_write", libcu8_write},
    {"_sopen", libcu8_sopen},
    {"_sopen", libcu8_sopen},
    {"_creat", libcu8_creat},
    {"_lseek", libcu8_lseek},
    {"_commit", libcu8_commit},
    {"_dup", libcu8_dup},
    {"_dup2", libcu8_dup2},
    {"_spawn", libcu8_spawnl},
    {"_spawnlp", libcu8_spawnlp}, /* 10th */
    {"_spawnlpe", libcu8_spawnlpe},
    {"_spawnle", libcu8_spawnle},
    {"_spawnv", libcu8_spawnv},
    {"_spawnve", libcu8_spawnve},
    {"_spawnvp", libcu8_spawnvp},
    {"_spawnvpe", libcu8_spawnvpe},
#ifdef HAVE__STAT
    {"_stat", libcu8_stat},
#endif
#ifdef HAVE__STAT32
    {"_stat32", libcu8_stat32},
#endif
#ifdef HAVE__STAT32I64
    {"_stat32i64", libcu8_stat32i64},
#endif
#ifdef HAVE__STAT64I32
    {"_stat64i32", libcu8_stat64i32}, /* 20th */
#endif
#ifdef HAVE__STAT64
    {"_stat64", libcu8_stat64},
#endif
#ifdef HAVE__FINDFIRST
    {"_findfirst", libcu8_findfirst},
    {"_findnext", libcu8_findnext},
#endif
#ifdef HAVE__FINDFIRST32
    {"_findfirst32", libcu8_findfirst32},
    {"_findnext32", libcu8_findnext32},
#endif
#ifdef HAVE__FINDFIRST32I64
    {"_findfirst32i64", libcu8_findfirst32i64},
    {"_findnext32i64", libcu8_findnext32i64},
#endif
#ifdef HAVE__FINDFIRST64
    {"_findfirst64", libcu8_findfirst64},
    {"_findnext64", libcu8_findnext64},
#endif
#ifdef HAVE__FINDFIRST64I32
    {"_findfirst64i32", libcu8_findfirst64i32}, /* 30th */
    {"_findnext64i32", libcu8_findnext64i32},
#endif
    {"_chdir", libcu8_chdir},
    {"_rmdir", libcu8_rmdir},
    {"_mkdir", libcu8_mkdir},
    {"_getcwd", libcu8_getcwd},
    {"_open", libcu8_open},
    {"fopen", libcu8_fopen},
    {"remove", libcu8_remove},
    {"rename", libcu8_rename},
    {"_unlink", libcu8_unlink}
};

#endif

/* initialize the new functions */
__LIBCU8__IMP __cdecl int libcu8_init(const char*** pargv)
{
    int i, n;
    HANDLE msvcrt;
    void* oldfn;

    /* Initialize the lock for fencoding */
    InitializeCriticalSection(&libcu8_fencoding_lock);
    InitializeCriticalSection(&libcu8_history_lock);
    EnterCriticalSection(&libcu8_fencoding_lock);

    sprintf(libcu8_fencoding, "UTF-8");
    libcu8_dummy = 1;

    LeaveCriticalSection(&libcu8_fencoding_lock);

    /* Allocate memory for libcu8 internal buffers */
    if (!(libcu8_fd_buffers = malloc(FD_BUFFERS_SIZE
                                        * sizeof(struct fd_buffering_t))))
        return -1;

    /* Reset buffers */
    for (i=0;i < FD_BUFFERS_SIZE; i++)
        libcu8_reset_buffered(i); /* reset all the buffers */

    /* Get utf-8 encoded argv */
    if (pargv != NULL && libcu8_get_argv(pargv) == -1 )
        return -1;

    return 0;

}

typedef struct {
int newmode;
} _startupinfo;

#if defined(_WIN32) && !defined(_MSC_VER)
extern int __wgetmainargs(int*, wchar_t***, wchar_t***, int, _startupinfo*);
#define HAS_GET_MAIN_ARGS	1
#endif

int libcu8_get_argv(const char*** pargv)
{
    wchar_t **wargv=NULL, **wenv=NULL;
    char **argv=NULL;
    int argc=0, i;
    size_t converted;
    _startupinfo stinfo;

#ifdef HAS_GET_MAIN_ARGS
    __wgetmainargs(&argc, &wargv, &wenv, 0, &stinfo);
#endif

    if (!(argv = (char**)malloc((argc + 1) * sizeof(char*)))) {

        errno = ENOMEM;
        return -1;

    }

    argv[argc] = NULL;

    for (i=0; i < argc; i ++) {

        if (!(argv[i] = libcu8_xconvert(LIBCU8_FROM_U16, (char*)wargv[i],
                                (wcslen(wargv[i])+1)*sizeof(wchar_t), &converted))) {

            return -1;

        }

    }

    *pargv = (const char**)argv;

    return 0;
}
