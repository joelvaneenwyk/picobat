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

#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#include "internals.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#include <io.h>
#include <share.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

FILE* _libcu8_wfopen(const wchar_t* filename, const wchar_t* mode) {
#ifdef _WIN32
    return _wfopen(filename, mode);
#else
    char filename_mb[1024];
    char mode_mb[16];
    wcstombs(filename_mb, filename, sizeof(filename_mb));
    wcstombs(mode_mb, mode, sizeof(mode_mb));
    return fopen(filename_mb, mode_mb);
#endif
}

int _libcu8_wopen(const wchar_t* filename, int oflag, int pmode) {
#ifdef _WIN32
    return _wopen(filename, oflag, pmode);
#else
    char filename_mb[1024];
    wcstombs(filename_mb, filename, sizeof(filename_mb));
    return open(filename_mb, oflag, pmode);
#endif
}

int _libcu8_wsopen(const wchar_t* filename, int oflag, int shflag, int pmode) {
#ifdef _WIN32
    return _wsopen(filename, oflag, shflag, pmode);
#else
    char filename_mb[1024];
    wcstombs(filename_mb, filename, sizeof(filename_mb));
    return open(filename_mb, oflag, pmode);
#endif
}

int _libcu8_wcreat(const wchar_t* filename, int pmode) {
#ifdef _WIN32
    return _wcreat(filename, pmode);
#else
    char filename_mb[1024];
    wcstombs(filename_mb, filename, sizeof(filename_mb));
    return creat(filename_mb, pmode);
#endif
}

int _libcu8_wremove(const wchar_t* filename) {
#ifdef _WIN32
    return _wremove(filename);
#else
    char filename_mb[1024];
    wcstombs(filename_mb, filename, sizeof(filename_mb));
    return remove(filename_mb);
#endif
}

int _libcu8_wunlink(const wchar_t* filename) {
#ifdef _WIN32
    return _wunlink(filename);
#else
    char filename_mb[1024];
    wcstombs(filename_mb, filename, sizeof(filename_mb));
    return unlink(filename_mb);
#endif
}

int _libcu8_wrename(const wchar_t* oldname, const wchar_t* newname) {
#ifdef _WIN32
    return _wrename(oldname, newname);
#else
    char oldname_mb[1024];
    char newname_mb[1024];
    wcstombs(oldname_mb, oldname, sizeof(oldname_mb));
    wcstombs(newname_mb, newname, sizeof(newname_mb));
    return rename(oldname_mb, newname_mb);
#endif
}

__LIBCU8__IMP __cdecl FILE* libcu8_fopen(const char* __restrict__ name, const char* __restrict__ mode)
{
    wchar_t *wfile, *wmode;
    int fd;
    size_t conv;
    FILE* ret;

    if (!(wfile = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, name,
                                                strlen(name)+1, &conv))) {

        errno = ENOMEM;
        return NULL;

    }

    if (!(wmode = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, mode,
                                                strlen(mode)+1, &conv))) {

        errno = ENOMEM;
        free(wfile);
        return NULL;

    }

    ret = _libcu8_wfopen(wfile, wmode);

    fd = fileno(ret);

    if (fd != -1) {

        /* empty buffering structure */
        libcu8_fd_buffers[fd].rcount = 0;
        libcu8_fd_buffers[fd].len = 0;

    }

    free(wfile);
    free(wmode);

    return ret;
}

__LIBCU8__IMP __cdecl int libcu8_open(char* name, int oflags, ...)
{
    wchar_t *wcs;
    int fd, pmode = 0;
    size_t len;
    va_list args;

    if (!(wcs = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, name, strlen(name)+1,
                                          &len)))
        return -1;

    if (oflags & O_CREAT) {

        va_start(args, oflags);
        pmode = va_arg(args, int);
        va_end(args);

    }

    fd = _libcu8_wopen(wcs, oflags, pmode);

    free(wcs);

    if (fd != -1) {

        /* empty buffering structure */
        libcu8_fd_buffers[fd].rcount = 0;
        libcu8_fd_buffers[fd].len = 0;

    }

    return fd;
}


__LIBCU8__IMP __cdecl int libcu8_sopen(char* name, int oflags, int shflags, int pmode)
{
    wchar_t *wcs;
    int fd;
    size_t len;

    if (!(wcs = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, name, strlen(name)+1,
                                          &len)))
        return -1;

    fd = _libcu8_wsopen(wcs, oflags, shflags, pmode);

    free(wcs);

    if (fd != -1) {

        /* empty buffering structure */
        libcu8_fd_buffers[fd].rcount = 0;
        libcu8_fd_buffers[fd].len = 0;

    }

    return fd;
}

__LIBCU8__IMP __cdecl int libcu8_creat(char* name, int pmode)
{
    wchar_t *wcs;
    int fd;
    size_t len;

    if (!(wcs = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, name, strlen(name)+1,
                                          &len)))
        return -1;

    fd = _libcu8_wcreat(wcs, pmode);

    free(wcs);

    if (fd != -1) {

        /* empty buffering structure */
        libcu8_fd_buffers[fd].rcount = 0;
        libcu8_fd_buffers[fd].len = 0;

    }

    return fd;

}

__LIBCU8__IMP __cdecl int libcu8_commit(int fd)
{
#ifdef _WIN32
    return _commit(fd);
#else
    return fsync(fd);
#endif

    libcu8_fd_buffers[fd].rcount = 0;
    libcu8_fd_buffers[fd].len = 0;

    return 0;
}

__LIBCU8__IMP __cdecl int libcu8_fflush(FILE* f)
{
    int fd = fileno(f);
    fflush(f);

    libcu8_fd_buffers[fd].rcount = 0;
    libcu8_fd_buffers[fd].len = 0;

    return 0;
}


__LIBCU8__IMP __cdecl int libcu8_lseek(int fd, long offset, int origin)
{
    int ret;

    ret = lseek(fd, offset, origin);

    libcu8_fd_buffers[fd].rcount = 0;
    libcu8_fd_buffers[fd].len = 0;

    return ret;
}

__LIBCU8__IMP __cdecl int libcu8_fseek(FILE* f, long offset, int origin)
{
    int ret, fd = fileno(f);

    ret = fseek(f, offset, origin);

    libcu8_fd_buffers[fd].rcount = 0;
    libcu8_fd_buffers[fd].len = 0;

    return ret;
}

__LIBCU8__IMP __cdecl int libcu8_fd_set_inheritance(int fd, int mode)
{
    HANDLE handle = (HANDLE)_get_osfhandle(fd);

    SetHandleInformation(handle, HANDLE_FLAG_INHERIT, mode);

    return 0;
}

/* CRT function replacement for file management functions */
__LIBCU8__IMP __cdecl int libcu8_remove(const char* file)
{
    wchar_t *wcs;
    int ret;
    size_t len;

    if (!(wcs = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, file, strlen(file)+1,
                                          &len)))
        return -1;

    ret = _libcu8_wremove(wcs);
    free(wcs);

    return ret;
}

__LIBCU8__IMP __cdecl int libcu8_unlink(const char* file)
{
    wchar_t *wcs;
    int ret;
    size_t len;

    if (!(wcs = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, file, strlen(file)+1,
                                          &len)))
        return -1;

    ret = _libcu8_wunlink(wcs);
    free(wcs);

    return ret;
}

__LIBCU8__IMP __cdecl int libcu8_rename(const char* oldn, const char* newn)
{
    wchar_t *wold=NULL, *wnew=NULL;
    int ret;
    size_t len;

    if ((wold = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, oldn, strlen(oldn)+1,
                                          &len))
        && (wnew = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, newn, strlen(newn)+1,
                                          &len))) {

        ret = _libcu8_wrename(wold, wnew);

    } else
        ret = -1;


    if (wold)
        free(wold);
    if (wnew)
        free(wnew);

    return ret;
}
