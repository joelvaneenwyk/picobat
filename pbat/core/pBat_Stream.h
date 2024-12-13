/*
 *
 *   pBat - A Free, Cross-platform command prompt - The pBat project
 *   Copyright (C) 2010-2016 Romain GARBI
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PBAT_MODULE_STREAM_H
#define PBAT_MODULE_STREAM_H

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libpBat.h>

/* used for compatibility purpose */
#if defined(_WIN32)
#	include <io.h>
#	ifndef access
#		define access _access
#	endif
#elif defined(WIN32)
#	include <unistd.h>
#endif /* WIN32 */

#define PBAT_STDIN                  0 /* redirect fInput */
#define PBAT_STDOUT                 1 /* redirect fOutput */
# define PBAT_STDERR                 2 /* redirect fError */
#define PBAT_STREAM_MODE_ADD        0 /* Open in add mode eg `>>`*/
#define PBAT_STREAM_MODE_TRUNCATE   4 /* Open in truncate mode eg. `>` */
#define PBAT_STREAM_SUBST_FOUTPUT   8 /* also copy the new fError to fOutput*/
#define PBAT_STREAM_SUBST_FERROR    16 /* also copy the fOutput to fError */
#define PBAT_STREAM_LOCKED          32 /* locking status of stack */

#define pBat_SetStreamStackLockState(stack, state) \
            ((stack) ? (stack->mode = (stack->mode & ~PBAT_STREAM_LOCKED) \
                                        | ((state != 0) <<  5)) : 0)

#define pBat_GetStreamStackLockState(stack) \
            ((stack) ? (stack->mode & PBAT_STREAM_LOCKED) : 1 )


/* structure used to store the state of stream redirections */
typedef struct STREAMSTACK {
    int mode; /* the file descriptor that has been redirected */
    FILE* old; /* a duplicate of the previous FILE* associated with fInput, fOutput, fError */
    FILE* subst; /* a operation of file substitution if either 2>&1 or 1>&2 is used */
    struct STREAMSTACK* previous;
} STREAMSTACK, *LPSTREAMSTACK;


/* On windows, 0 is an invalid argument to setvbuf() */
#ifdef WIN32
#define PBAT_DEF_BUF 256
#else
#define PBAT_DEF_BUF 0
#endif


#define PBAT_RESET_BUFFERING(fd, s) \
            if (isatty(fd)) \
                setvbuf(s, NULL, _IONBF, 0); \
            else \
                setvbuf(s, NULL, _IOFBF, PBAT_DEF_BUF)

#define __PBAT_DUP_STD(fd, s) \
            if (dup2(fd, fileno(s)) == -1) \
                pBat_ShowErrorMessage(PBAT_UNABLE_DUPLICATE_FD \
                                        | PBAT_PRINT_C_ERROR, \
                                            __FILE__ "/PBAT_DUP_STD()", -1);

#define PBAT_DUP_STD(fd, s) \
            fflush(s);\
            __PBAT_DUP_STD(fd, s) \
            PBAT_RESET_BUFFERING(fd, s)

/* The trick here is that ftell(s) takes account of buffering situation so that
   fseek can set positions accurately, but however, the FILE structure does not
   contains any absolute counter for file pos.

   Hence here we reset the postion of the file descriptor and then use fflush()
   to clear any buffering information still in the FILE */
#define PBAT_DUP_STDIN(fd, s) \
            if (!isatty(fileno(s))) \
                lseek(fileno(s), ftell(s), SEEK_SET); \
            fflush(s); \
            __PBAT_DUP_STD(fd, s) \
            PBAT_RESET_BUFFERING(fd, s)

#define PBAT_XDUP(fd, s) \
            if (((fd) = dup(fileno(s))) == -1) \
                pBat_ShowErrorMessage(PBAT_UNABLE_DUPLICATE_FD \
                                        | PBAT_PRINT_C_ERROR, \
                                            __FILE__ "/PBAT_DUP_STD()", -1);


/* initializes the stream stack */
STREAMSTACK* pBat_InitStreamStack(void);

/* frees the stream stack */
void pBat_FreeStreamStack(STREAMSTACK* stack);

/* Duplicate file based on a file name or a file descriptor */
STREAMSTACK* pBat_OpenOutput(STREAMSTACK* stack, char* name, int mode);
STREAMSTACK* pBat_OpenOutputF(STREAMSTACK* stack, FILE* f, int mode);

/* Pop stream stack functions */
STREAMSTACK* pBat_PopStreamStack(STREAMSTACK* stack);
STREAMSTACK* pBat_PopStreamStackUntilLock(STREAMSTACK* stack);

void pBat_ApplyStreams(STREAMSTACK* stack);
void pBat_UnApplyStreams(STREAMSTACK* stack);
#endif
