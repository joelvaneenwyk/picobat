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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>

#include <libpBat.h>

#include "../core/pBat_Core.h"

#include "pBat_Cd.h"

#include "../lang/pBat_Lang.h"
#include "../lang/pBat_ShowHelp.h"

// #define PBAT_DBG_MODE
#include "../core/pBat_Debug.h"

#include "../errors/pBat_Errors.h"

/* Changes the system's current directory

    CD [/d] [[drive:]path]                    (Windows)
    CD [path]                                 (Unixes)

    Changes the system current directory.

        - [path] : The path for the new current directory.

    ** Windows specific **

        - [/d] : If the combination drive:path is located in another
        directory, force drive change. If drive:path is located in another
        directory, but /d is not specified, then it changes the current
        directory on the drive, without changing the current directory on
        that drive.

    Current directory on drive are stored in environment variables (undocumented,
    but cmd-compatible), that are named following the rule : %drive=:%.

    Similar effects may be achieved using the popd and pushd in unixes commands.

    ** End Windows specific **

    NOTE a little inconsistency with cmd.exe. %CD% *never* ends with a '\' on
    windows ! This allows doing proper treatment when being a disk root ...
 */

int __inline__ pBat_Canonicalize(char* path)
{
    char *previous = NULL, *next, *orig = path;
    size_t size = strlen(path) + 1;

    /* If the path is UNC, just skip the very first part */
    if (PBAT_TEST_UNC_PATH(path)) {
        path += 2;

	while (*path && !PBAT_TEST_SEPARATOR(path))
		path ++;

    }

    /* first pass to clean multiple separators and "." characters */
    while (*path) {

        if (PBAT_TEST_SEPARATOR(path))
            *path = PBAT_DEF_SEPARATOR;

        if (PBAT_TEST_SEPARATOR(path)) {

            /* Try to swallow multiple delimiters */
            next = path + 1;

reloop:
            while (PBAT_TEST_SEPARATOR(next))
                next ++;

            /* check this not a dull "." folder */
            if (*next == '.' && PBAT_TEST_SEPARATOR((next + 1))) {
                next ++;
                goto reloop;
            } else if (*next == '.' && *(next + 1) == '\0')
                next ++;

            if (next != path + 1) {
                memmove(path + 1, next, size - (next - orig));
                size -= next - (path + 1); /* remove bytes that were swallowed */
            }

            if (*(path + 1) == '.' && *(path + 2) == '.'
                && (PBAT_TEST_SEPARATOR((path + 3)) || *(path + 3) == '\0')) {

                /* Apparently, this folder is "..", find the previous dir & swallow both */
                previous = path - 1;
                while (previous >= orig && !PBAT_TEST_SEPARATOR(previous))
                    previous --;

                if (previous < orig) {

                    /* this is not meant to happen unless if .. follows directly the root
                       remove it then */
#ifdef WIN32
                    *path = '\0';
#else
                    *(path + 1) = '\0';
#endif // WIN32

                } else {

                    /* squeeze the preceding dir and the separator */
                    memmove(previous, path+3, size - ((path + 3 ) - orig));
                    size -= (path + 3) - previous;
                    path = previous;
                    continue;

                }

            }

            if (*path && *(path + 1) == '\0' && path != orig) {
                *path = '\0';
                break;
            }

        }

        if (*path)
            path ++;
    }

    /* Wait ... */

    return 0;
}

int pBat_SetCurrentDir(char* lpLine)
{
    char *p/*, *p2 = NULL */;
    size_t remain;

    if (PBAT_TEST_ABSOLUTE_PATH(lpLine)
        && pBat_DirExists(lpLine)) {

 #ifdef WIN32
        /* Under windows, we do not have unique file system
           root, so that '/' refers to the current active drive, we have
           to deal with it */

        if (*lpLine == '/') {

            /* well, arguably lpCurrentDir[2] is a slash ...
               unless lpCurrentDir refers to a unc path
               which is still unsupported  */

            strncpy(lpCurrentDir + 2, lpLine, FILENAME_MAX-2);
            lpCurrentDir[FILENAME_MAX-1] = '\0';

            return pBat_Canonicalize(lpCurrentDir);
        }
 #endif

        strncpy(lpCurrentDir, lpLine, FILENAME_MAX);
        lpCurrentDir[FILENAME_MAX - 1] = '\0';
        return pBat_Canonicalize(lpCurrentDir);

    }

    /* find the end of the current dir */
    p = lpCurrentDir;
    while (*p)
        p ++;

#ifdef WIN32
    *p = '\\';
#else
    *p = '/';
#endif

    remain = FILENAME_MAX - (p + 1 - lpCurrentDir);

    if (remain > 0) {

        strncpy(p + 1, lpLine, remain);
        lpCurrentDir[FILENAME_MAX - 1] = '\0';

        if (pBat_DirExists(lpCurrentDir))
            return pBat_Canonicalize(lpCurrentDir);

    }

    *p = '\0';

    errno = ENOENT;
    return -1;
}

int pBat_CmdCd_nix(char* lpLine)
{
	char* lpNext;
	ESTR* lpEsDir=pBat_EsInit_Cached();
	int status = PBAT_NO_ERROR,
	    quotes = 0;

	if (!strnicmp(lpLine,"CD", 2)){
		lpLine+=2;
	} else if (!strnicmp(lpLine, "CHDIR", 5)) {
		lpLine+=5;
	}

	if ((lpNext=pBat_GetNextParameterEs(lpLine, lpEsDir))) {

		if (!strcmp(pBat_EsToChar(lpEsDir), "/?")) {

			pBat_ShowInternalHelp(PBAT_HELP_CD);
			goto error;

		} else if (!stricmp(pBat_EsToChar(lpEsDir), "/d")) {

			lpLine=lpNext;

		}

		while (*lpLine==' ' || *lpLine=='\t') lpLine++;

		pBat_GetEndOfLine(lpLine, lpEsDir);

		lpLine=pBat_SkipBlanks(pBat_EsToChar(lpEsDir));

        if (*lpLine == '"') {

            quotes = 1;
            lpLine ++;

        }

        while (*lpLine) {

            switch(*lpLine) {
                case '"':
                    if (!quotes)
                        goto def;
                case '\t':
                case ' ':

                    if (!lpNext) lpNext=lpLine;
                    break;
def:
                default:
                    lpNext=NULL;
            }

            lpLine++;

        }

        if (lpNext)
            *lpNext = '\0';


		errno=0;

		lpLine=pBat_SkipBlanks(pBat_EsToChar(lpEsDir));

		if (quotes)
            lpLine ++;

		PBAT_DBG("Changing directory to : \"%s\"\n", lpLine);

		if (pBat_SetCurrentDir(lpLine)) {


			/* do not perform errno checking
			   as long as the most important reason for
			   chdir to fail is obviously the non existence
			   or the specified directory

			   However, it appears that this is inconsistant
			   using windows as it does not returns on failure
			   every time a non-existing folder is passed to the
			   function, tried with '.. ' on my system

			*/

			pBat_ShowErrorMessage(PBAT_DIRECTORY_ERROR, lpLine, FALSE);

            status = PBAT_DIRECTORY_ERROR;
			goto error;

		}

	} else {

		fputs(lpCurrentDir, fOutput);

	}

error:
	pBat_EsFree_Cached(lpEsDir);
	return status;
}


int pBat_CmdCd_win(char* lpLine)
{
    char varname[]="=x:",
		*lpNext,
		 current=*lpCurrentDir,
		 passed=0;
    int quotes = 0;

    ESTR* lpesStr=pBat_EsInit_Cached();

    int force=0,
		status=0;

    if (!strnicmp("cd", lpLine, 2))
        lpLine +=2;
    else if (!strnicmp("chdir", lpLine, 5))
        lpLine +=5;


    if (!(lpNext = pBat_GetNextParameterEs(lpLine, lpesStr))) {

        fputs(lpCurrentDir, fOutput);
        fputs(PBAT_NL, fOutput);

        status = 0;

        goto end;

    }

    if (!stricmp(pBat_EsToChar(lpesStr), "/D")) {

        lpLine = lpNext;
        force = TRUE;

    }

    pBat_GetEndOfLine(lpLine, lpesStr);

    lpLine = pBat_SkipBlanks(lpesStr->str);
    lpNext = NULL;

    if (*lpLine == '"') {

        quotes = 1;
        lpLine ++;

    }

    while (*lpLine) {

        switch(*lpLine) {
            case '"':
                if (!quotes)
                    goto def;
            case '\t':
            case ' ':

                if (!lpNext) lpNext=lpLine;
                break;
def:
            default:
                lpNext=NULL;
        }

        lpLine++;

    }

    if (lpNext)
        *lpNext = '\0';

    lpLine = pBat_SkipBlanks(pBat_EsToChar(lpesStr));

    if (quotes)
        lpLine ++;

    if (*lpLine && *(lpLine+1)==':') {

        if (*pBat_SkipBlanks(lpLine+2) == '\0') {

            /* If the only  argument is a drive letter eg `x:` do the following
               steps :
                    - check if `=x:` variable exists, if so change current directory to it
                    - if no change directory to x:\ and set `=x:` accordingly */

            force = TRUE;

            varname[1] = *lpLine;

            if (!(lpLine = pBat_GetEnv(lpeEnv, varname))) {

                /* append a slash so this becomes a valid path */
                lpLine = pBat_SkipBlanks(lpesStr->str);

            }
        }

        passed = *lpLine;

    }

    if ((passed == 0)
        || (toupper(passed) == toupper(current))
        || force) {

        /* change the current directory, yeah */

        if (pBat_SetCurrentDir(lpLine)) {

            pBat_ShowErrorMessage(PBAT_DIRECTORY_ERROR | PBAT_PRINT_C_ERROR,
                                    lpLine,
                                    FALSE
                                    );

            status = PBAT_DIRECTORY_ERROR;
            goto end;

        }

    }

    varname[1] = (passed == 0) ? (current) : (passed);
    pBat_SetEnv(lpeEnv, varname, (passed == 0) ? lpCurrentDir : lpLine);

end:
    pBat_EsFree_Cached(lpesStr);

    return status;
}
