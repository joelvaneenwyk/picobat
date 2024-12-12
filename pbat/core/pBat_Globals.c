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

#include "libpBat.h"
#include "pBat_Core.h"
#include "../../config.h"

int iMainThreadId;
char* lpInitVar[]= {
	"PBAT_VERSION", PBAT_VERSION,
	"PBAT_OS", PBAT_OS,
	NULL, NULL, /* PBAT_PATH is dinamically generated */
#ifdef WIN32
	NULL, NULL, /* used to initialize the %=x:% variable based on the current path*/
#endif
    "PBAT_OS_TYPE", PBAT_OS_TYPE,
    "PBAT_START_SCRIPT", START_SCRIPT,
    "PROMPT","$P$G",
    "NUL", NUL,
    "CON", CON,
    "COM", COM,
    "PRN", PRN,
    "LPT", LPT,
	NULL, NULL
};

void(*pErrorHandler)(void)=NULL;
int fdStdin;
int fdStdout;
int fdStderr;
MUTEX mThreadLock;
MUTEX mSetLock; /* a lock to serialize calls to yacc-generated parsers */
MUTEX mRunFile; /* a lock to serialize calls to create process */
MUTEX mEchoLock; /* a lock to serialize calls to echo */
MUTEX mModLock; /* a lock to serialize calls to mod */
char lppBatPath[FILENAME_MAX]; /* A path to the directory of the pBat executable */
char lppBatExec[FILENAME_MAX]; /* A path to the pBat executable */

THREAD_LOCAL int bDelayedExpansion=TRUE;
THREAD_LOCAL int bEchoOn=TRUE;

/* call exit or endthread uppon exit*/
THREAD_LOCAL int bIgnoreExit=FALSE;

/* Current errorlevel value */
THREAD_LOCAL int iErrorLevel=0;
THREAD_LOCAL int bIsScript;
THREAD_LOCAL int bCmdlyCorrect=FALSE;

THREAD_LOCAL int bAbortCommand=FALSE;
THREAD_LOCAL LPCOMMANDLIST lpclCommands;
THREAD_LOCAL LOCAL_VAR_BLOCK* lpvLocalVars;
    /* use a distinct local block for command arguments

        %1-%9 : arguments
        %* : full line
        %+ : remaning args */
THREAD_LOCAL LOCAL_VAR_BLOCK* lpvArguments;
THREAD_LOCAL LPSTREAMSTACK lppsStreamStack;
THREAD_LOCAL struct dirstack_t dsDirStack; /* current directory stack */

THREAD_LOCAL COLOR colColor=PBAT_COLOR_DEFAULT;
THREAD_LOCAL FILE* fInput; /* current thread input stream */
THREAD_LOCAL FILE *fOutput; /* current thread output stream */
THREAD_LOCAL FILE *fError; /* current thread error stream */
/* Note : the underscore prefixed version are internally used to
   handle output substitution (eg. 2>&1) to be used as backup and
   so prevent unnecessary duplication of files */

THREAD_LOCAL ENVBUF* lpeEnv;
THREAD_LOCAL ENVSTACK* lpesEnv = NULL;
THREAD_LOCAL INPUT_FILE ifIn;
THREAD_LOCAL char lpCurrentDir[FILENAME_MAX];
THREAD_LOCAL ESTRCACHE ecEstrCache; /* ESTR cache */

THREAD_LOCAL char* lpAltPromptString = NULL;

#ifdef WIN32
#define environ _environ
#else
extern char** environ;
MUTEX mLineNoise;
#endif
