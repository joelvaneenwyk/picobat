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
#ifndef LIBCU8_H
#define LIBCU8_H

#ifndef __LIBCU8__IMP
#ifdef __LIBCU8__DLL
#define __LIBCU8__IMP __declspec(dllexport)
#else
#define __LIBCU8__IMP extern
#endif /* __LIBCU8__DLL */
#endif /* __LIBCU8__IMP */

/* enable C++ compatibility */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <sys/stat.h>

/* for WC_NO_BEST_FIT_CHARS */
#ifndef WINVER
#define WINVER 0x0500
#endif

#ifndef STRICT
#define STRICT
#endif

#if defined(_WIN32) || defined(WIN32) || defined(__MINGW32__) || defined(__MINGW64__)
	#include <windows.h>

	#ifndef TEXT
	#define TEXT(text) text
	#endif

	#ifndef LoadLibrary
	#define LoadLibrary(name) LoadLibraryA(name)
	#endif

	#define _stricmp _stricmp
	#define _strnicmp _strnicmp
#else
	#include <dlfcn.h>
	#include <strings.h>

	#ifndef TEXT
	#define TEXT(text) text
	#endif

	#ifndef LoadLibrary
	#define LoadLibrary(name) dlopen(name, RTLD_LAZY)
	#endif

	#define stricmp(a, b) strcasecmp(a, b)
	#define strnicmp(a , b, c) strncasecmp(a, b, c)

	#define _stricmp strcasecmp
	#define _strnicmp strncasecmp
#endif

#if !defined(__linux__)
#	include <io.h>
#	include <windows.h>
#else
#	include <unistd.h>
#	include <stdint.h>
#	include <string.h>
#	include <dirent.h>
#endif	/* defined(WIN32) */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if !defined(__cdecl)
    #define __cdecl
#endif

#if defined(__x86_32__) || defined(__i386__) || defined(_M_IX86) || defined(__32BIT__)
#	define __LIBCU8_IS_32BIT_PLATFORM
#endif

#if !defined(WIN32) && !defined(_MINWINDEF_)
typedef char CHAR;
typedef char* LPSTR;
typedef unsigned char* LPBYTE;
typedef const char* LPCCH;
typedef const char* LPCSTR;
typedef const void* LPCVOID;
typedef const wchar_t* LPCWCH;
typedef const wchar_t* LPCWSTR;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef unsigned int* LPINT;
typedef unsigned long DWORD;
typedef unsigned long ULONG_PTR;
typedef unsigned long* LPDWORD;
typedef unsigned short WORD;

typedef int BOOL;
typedef BOOL* LPBOOL;

typedef long LONG;
typedef short SHORT;
typedef void* FARPROC;
typedef void* HANDLE;
typedef void* HMODULE;
typedef wchar_t WCHAR;
typedef wchar_t* LPWSTR;
typedef void* PVOID;

typedef void* LPSECURITY_ATTRIBUTES;
typedef struct _RTL_CRITICAL_SECTION_DEBUG *PRTL_CRITICAL_SECTION_DEBUG;
typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union {
        struct {
            DWORD Offset;
            DWORD OffsetHigh;
        } DUMMYSTRUCTNAME;
        PVOID Pointer;
    } DUMMYUNIONNAME;
    HANDLE hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef DWORD LCID;

#define O_CREAT  0x0100
#define O_WRONLY 0x0001  // Open for write-only access
#define O_TRUNC  0x0200  // Truncate file to zero length

#define ERROR_NO_MORE_FILES 18L

#define FALSE 0
#define TRUE 1

#define HANDLE_FLAG_INHERIT 0x00000001
#define KEY_EVENT 0x0001
#define VK_DELETE 0x2E
#define VK_DOWN 0x28
#define VK_END 0x23
#define VK_HOME 0x24
#define VK_LEFT 0x25
#define VK_RIGHT 0x27
#define VK_UP 0x26

#define MAX_PATH 260

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#define STD_ERROR_HANDLE ((DWORD)-12)
#define STD_OUTPUT_HANDLE ((DWORD)-11)

typedef long HRESULT;
#define S_OK ((HRESULT)0L)

#ifdef WIN32
#define WINAPI __stdcall
#else
#define WINAPI
#endif

#define GENERIC_WRITE O_WRONLY
#define CREATE_ALWAYS (O_CREAT | O_TRUNC)
#define FILE_ATTRIBUTE_NORMAL 0
#define HANDLE int
#define INVALID_HANDLE_VALUE -1
#define DWORD ssize_t

#define MAX_DEFAULTCHAR 2
#define MAX_LEADBYTES 12
#define MB_ERR_INVALID_CHARS 0x00000008
#define ERROR_INSUFFICIENT_BUFFER 122L

#	ifdef _WIN64
typedef __int64 LONG_PTR;
#	else
typedef long LONG_PTR;
#	endif

/** A structure to store console coordinates */
typedef struct COORD {
	short X; /**< The x coordinate (column number). Starts at 0 */
	short Y; /**< The y coordinate (line number). Starts at 0 */
} COORD, *LPCOORD;

typedef struct _cpinfo {
    UINT MaxCharSize;                // Maximum length (in bytes) of a character in the code page
    BYTE DefaultChar[MAX_DEFAULTCHAR]; // Default character used when a character cannot be represented in the code page
    BYTE LeadByte[MAX_LEADBYTES];    // Array of lead byte ranges
} CPINFO, *LPCPINFO;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT;

typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    WORD  wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO;

typedef struct _RTL_CRITICAL_SECTION {
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;
    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;
} CRITICAL_SECTION, *PCRITICAL_SECTION;

typedef struct _CONSOLE_CURSOR_INFO {
    DWORD dwSize;
    BOOL bVisible;
} CONSOLE_CURSOR_INFO;

typedef struct _INPUT_RECORD {
    WORD EventType;
    union {
        struct {
            BOOL bKeyDown;
            WORD wRepeatCount;
            WORD wVirtualKeyCode;
            WORD wVirtualScanCode;
            union {
                WCHAR UnicodeChar;
                CHAR AsciiChar;
            } uChar;
            DWORD dwControlKeyState;
        } KeyEvent;
        // Other event types omitted for brevity
    } Event;
} INPUT_RECORD;

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME;

typedef struct _WIN32_FIND_DATAW {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    WCHAR cFileName[MAX_PATH];
    WCHAR cAlternateFileName[14];
} WIN32_FIND_DATAW, *PWIN32_FIND_DATAW;
#endif

/* initialization function */
__LIBCU8__IMP __cdecl int libcu8_init(const char*** pargv);

/* encoding management functions */
__LIBCU8__IMP __cdecl void libcu8_get_fencoding(char* enc, size_t size);
__LIBCU8__IMP __cdecl int libcu8_set_fencoding(const char* enc);

/* conversion modes for libcu8_convert and libcu8_xconvert */
#define LIBCU8_TO_U16       0
#define LIBCU8_FROM_U16     1
#define LIBCU8_TO_ANSI      2
#define LIBCU8_FROM_ANSI    3

/* Convert function */
__LIBCU8__IMP __cdecl char* libcu8_convert(int mode, const char* src,
                                            size_t size, char* remainder,
                                            size_t* rcount, size_t rlen,
                                            size_t* converted);
__LIBCU8__IMP __cdecl char* libcu8_xconvert(int mode, const char* src,
                                            size_t size, size_t* converted);

/* CRT function replacement for low-level io */
#ifndef LIBCU8_NO_WRAPPERS
#undef fgetc
#undef getc
#define fgetc(a) libcu8_fgetc(a)
#define getc(a) libcu8_fgetc(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_fgetc(FILE *f);

#ifndef LIBCU8_NO_WRAPPERS
#undef fgets
#define fgets(a,b,c) libcu8_fgets(a,b,c)
#endif
__LIBCU8__IMP __cdecl char* libcu8_fgets(char* buf, int s, FILE* f);

#ifndef LIBCU8_NO_WRAPPERS
#undef fprintf
#define fprintf libcu8_fprintf
#endif
__LIBCU8__IMP __cdecl int libcu8_fprintf(FILE* f, const char* fmt, ...);

#ifndef LIBCU8_NO_WRAPPERS
#undef vfprintf
#define vfprintf libcu8_vfprintf
#endif
__LIBCU8__IMP __cdecl int libcu8_vfprintf(FILE* f, const char* fmt, va_list args);

#ifndef LIBCU8_NO_WRAPPERS
#undef fputc
#undef putc
#define fputc(a,b) libcu8_fputc(a,b)
#define putc(a,b) libcu8_fputc(a,b)
#endif
__LIBCU8__IMP __cdecl int libcu8_fputc(int c, FILE* f);

#ifndef LIBCU8_NO_WRAPPERS
#undef fputs
#define fputs(a,b) libcu8_fputs(a,b)
#endif
__LIBCU8__IMP __cdecl int libcu8_fputs(const char* s, FILE* f);

#ifndef LIBCU8_NO_WRAPPERS
#undef fopen
#define fopen(a,b) libcu8_fopen(a,b)
#endif
__LIBCU8__IMP __cdecl FILE* libcu8_fopen(const char* __restrict__ name, const char* __restrict__ mode);

#ifndef LIBCU8_NO_WRAPPERS
#undef open
#define open libcu8_open
#endif
__LIBCU8__IMP __cdecl int libcu8_open(char* name, int oflags, ...);

#ifndef LIBCU8_NO_WRAPPERS
#undef sopen
#define sopen(a,b,c,d) libcu8_sopen(a,b,c,d)
#endif
__LIBCU8__IMP __cdecl int libcu8_sopen(char* name, int oflags,
                                            int shflags, int pmode);

#ifndef LIBCU8_NO_WRAPPERS
#undef creat
#define creat(a,b) libcu8_creat(a,b)
#endif
__LIBCU8__IMP __cdecl int libcu8_creat(char* name, int pmode);

#ifndef LIBCU8_NO_WRAPPERS
#undef lseek
#define lseek(a,b,c) libcu8_lseek(a,b,c)
#endif
__LIBCU8__IMP __cdecl int libcu8_lseek(int fd, long offset, int origin);

#ifndef LIBCU8_NO_WRAPPERS
#undef fseek
#define fseek(a,b,c) libcu8_fseek(a,b,c)
#endif
__LIBCU8__IMP __cdecl int libcu8_fseek(FILE* f, long offset, int origin);

#ifndef LIBCU8_NO_WRAPPERS
#undef commit
#define commit(a) libcu8_commit(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_commit(int fd);

#ifndef LIBCU8_NO_WRAPPERS
#undef fflush
#define fflush(a) libcu8_fflush(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_fflush(FILE* f);

#ifndef LIBCU8_NO_WRAPPERS
#undef dup
#define dup(a) libcu8_dup(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_dup(int fd);

#ifndef LIBCU8_NO_WRAPPERS
#undef dup2
#define dup2(a,b) libcu8_dup2(a,b)
#endif
__LIBCU8__IMP __cdecl int libcu8_dup2(int fd1, int fd2);

/* CRT function replacement for file management functions */
#ifndef LIBCU8_NO_WRAPPERS
#undef remove
#define remove(a) libcu8_remove(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_remove(const char* file);

#ifndef LIBCU8_NO_WRAPPERS
#undef rename
#define rename(a,b) libcu8_rename(a,b)
#endif
__LIBCU8__IMP __cdecl int libcu8_rename(const char* oldn, const char* newn);

#ifndef LIBCU8_NO_WRAPPERS
#undef unlink
#define unlink(a) libcu8_unlink(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_unlink(const char* file);

#ifndef LIBCU8_NO_WRAPPERS
#undef chmod
#define chmod(a,b) libcu8_chmod(a,b)
#endif
__LIBCU8__IMP __cdecl int libcu8_chmod(const char* file, int mode);

#ifndef LIBCU8_NO_WRAPPERS
#undef getcwd
#define getcwd(a,b) libcu8_getcwd(a,b)
#endif
__LIBCU8__IMP __cdecl int libcu8_getcwd(char* dir, size_t size);

#ifndef LIBCU8_NO_WRAPPERS
#undef chdir
#define chdir(a) libcu8_chdir(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_chdir(const char* dir);

#ifndef LIBCU8_NO_WRAPPERS
#undef rmdir
#define rmdir(a) libcu8_rmdir(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_rmdir(const char* dir);

#ifndef LIBCU8_NO_WRAPPERS
#undef mkdir
#define mkdir(a) libcu8_mkdir(a)
#endif
__LIBCU8__IMP __cdecl int libcu8_mkdir(const char* dir);

/* CRT functions replacement for spawn*/
__LIBCU8__IMP __cdecl intptr_t libcu8_spawnl(int mode, const char* file, ...);
__LIBCU8__IMP __cdecl intptr_t libcu8_spawnle(int mode, const char* file, ...);
__LIBCU8__IMP __cdecl intptr_t libcu8_spawnlp(int mode, const char* file, ...);
__LIBCU8__IMP __cdecl intptr_t libcu8_spawnlpe(int mode, const char* file, ...);
__LIBCU8__IMP __cdecl intptr_t libcu8_spawnv(int mode, const char* file,
                                                    const char* const *argv);
__LIBCU8__IMP __cdecl intptr_t libcu8_spawnve(int mode, const char* file,
                            const char* const *argv, const char *const *envp);
__LIBCU8__IMP __cdecl intptr_t libcu8_spawnvp(int mode, const char* file,
                                                const char* const *argv);
__LIBCU8__IMP __cdecl intptr_t libcu8_spawnvpe(int mode, const char* file,
                                                    const char* const *argv,
                                                    const char *const *envp);


__LIBCU8__IMP __cdecl int libcu8_fd_set_inheritance(int fd, int mode);

__LIBCU8__IMP __cdecl void (*libcu8_completion_handler)(const char*, char**);
__LIBCU8__IMP __cdecl void (*libcu8_completion_handler_free)(char*);

typedef void (*libcu8_completion_handler_t)(const char*, char**);
typedef void (*libcu8_completion_handler_free_t)(char*);

#ifdef dirent
#undef dirent
#endif

#ifdef DIR
#undef DIR
#endif

#ifdef opendir
#undef opendir
#endif

#ifdef closedir
#undef closedir
#endif

#ifdef readdir
#undef readdir
#endif


#ifndef HANDLE
typedef void* HANDLE;
#endif

struct libcu8_dirent {
    char* d_name;
    int ret;
};

typedef struct libcu8_DIR {
    HANDLE h;
    struct libcu8_dirent ent;
} libcu8_DIR;

__LIBCU8__IMP __cdecl libcu8_DIR* libcu8_opendir(const char* dir);
__LIBCU8__IMP __cdecl int libcu8_closedir(libcu8_DIR* pdir);
__LIBCU8__IMP __cdecl struct libcu8_dirent* libcu8_readdir(libcu8_DIR* pdir);

#define DIR libcu8_DIR
#define dirent libcu8_dirent

#ifndef __LIBCU8__DLL
#define readdir(pdir) libcu8_readdir(pdir)
#define closedir(pdir) libcu8_closedir(pdir)
#define opendir(dir) libcu8_opendir(dir)
#endif

/* enable c++ compatibility */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* INTERNALS_H */
