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
 *
 */

#ifndef PBAT_GLOBALS_H
#define PBAT_GLOBALS_H

#include <setjmp.h>
#include <stdio.h>
#include <libpBat.h>

#include "pBat_Core.h"
#include "../../config.h"

#ifndef WIN32
extern char** environ;
extern MUTEX mLineNoise;
#endif

/* static variable for initialization */
extern int iMainThreadId; /* thread id of the main thread */
extern void(*pErrorHandler)(void); /* error handler */
extern char* lpInitVar[]; /* list of variable for initialization */
extern MUTEX mThreadLock; /* a mutex for single threaded parts */
extern MUTEX mRunFile; /* a mutex to serialize call to fork() / CreateProcess() */
extern MUTEX mSetLock; /* a mutex for set (floats) */
extern MUTEX mEchoLock; /* a mutex for serializing calls to ECHO */
extern MUTEX mModLock; /* a lock to serialize calls to mod */

extern int fdStdin; /* temporary storage for streams */
extern int fdStdout;
extern int fdStderr;

extern char lppBatPath[]; /* A path to the directory of the pBat executable */
extern char lppBatExec[]; /* A path to the pBat executable */

/* Current state of the interpreter associated to this
   thread */
extern THREAD_LOCAL int bAbortCommand; /* abort the command (0: no , 1: jump to next l,
                                    -1 : move to upper execution level) */

#define PBAT_ABORT_EXECUTION_LEVEL -1
#define PBAT_ABORT_COMMAND_LINE 1
#define PBAT_ABORT_COMMAND_BLOCK 2

extern THREAD_LOCAL int bIgnoreExit; /* ignore exit */
extern THREAD_LOCAL int bDelayedExpansion; /* state of the delayed expansion */
extern THREAD_LOCAL int bCmdlyCorrect; /* state of cmdly correct interface */
extern THREAD_LOCAL int bIsScript; /* are we running a script ? */
extern THREAD_LOCAL int bEchoOn; /* is echo on ? */
extern THREAD_LOCAL int iErrorLevel; /* errorlevel state */
extern THREAD_LOCAL LPCOMMANDLIST lpclCommands; /* binary tree of commands */
extern THREAD_LOCAL LOCAL_VAR_BLOCK* lpvLocalVars; /* local variables array */
extern THREAD_LOCAL LOCAL_VAR_BLOCK* lpvArguments; /* arguments array */
extern THREAD_LOCAL LPSTREAMSTACK lppsStreamStack; /* status associated with streams */
extern THREAD_LOCAL struct dirstack_t dsDirStack; /* current directory stack. */
extern THREAD_LOCAL COLOR colColor; /* current command prompt colors */
extern THREAD_LOCAL INPUT_FILE ifIn; /* current parsed script */
extern THREAD_LOCAL ENVBUF* lpeEnv; /* environment variables local to threads */
extern THREAD_LOCAL FILE *fInput; /* current thread input stream */
extern THREAD_LOCAL FILE *fOutput; /* current thread output stream */
extern THREAD_LOCAL FILE *fError; /* current thread error stream */
extern THREAD_LOCAL ENVSTACK* lpesEnv;
extern THREAD_LOCAL char lpCurrentDir[FILENAME_MAX]; /* current path */
extern THREAD_LOCAL ESTRCACHE ecEstrCache; /* ESTR cache */

extern THREAD_LOCAL char* lpAltPromptString; /* possible alternate prompt string for completion */

#endif
