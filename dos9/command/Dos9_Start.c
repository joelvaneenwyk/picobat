/*
 *
 *   Dos9 - A Free, Cross-platform command prompt - The Dos9 project
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
#include <string.h>

#if !defined(WIN32)
#include <sys/wait.h>
#endif

#include <libDos9.h>

#include "Dos9_Start.h"
#include "../core/Dos9_Core.h"

#include "../errors/Dos9_Errors.h"
#include "../lang/Dos9_ShowHelp.h"

/*

	start [/Wait] [/MIN] [/MAX] [/b] [/D dir] cmd param

 */



#if defined(WIN32)

#include <windows.h>
#include <shellapi.h>

#define SEE_MASK_NOASYNC 0x00000100

#if defined(DOS9_USE_LIBCU8)
#include <libcu8.h>

int Dos9_StartFile(const char* file, const char* args, const char* dir,
					int mode, int wait)
{
	SHELLEXECUTEINFOW info;
	wchar_t *chr;
	size_t cvt;

	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask =  SEE_MASK_NOASYNC |
        ((mode & START_MODE_BACKGROUND) ? (SEE_MASK_NO_CONSOLE) : (0));
	info.lpVerb = NULL;
	info.lpDirectory = NULL;
	info.nShow = mode & ~START_MODE_BACKGROUND;

    if (!(info.lpFile = libcu8_xconvert(LIBCU8_TO_U16, file,
                                        strlen(file) + 1, &cvt))
        || !(info.lpParameters = libcu8_xconvert(LIBCU8_TO_U16, args,
                                                    strlen(args)+1, &cvt))) {

            if (info.lpFile)
                free(info.lpFile);

            if (info.lpParameters)
                free(info.lpParameters);

            if (info.lpDirectory)
                free(info.lpDirectory);

            Dos9_ShowErrorMessage(DOS9_FAILED_ALLOCATION, "libcu8_xconvert()", FALSE);

            return -1;
    }

    if (dir) {

        if  (!(info.lpDirectory = libcu8_xconvert(LIBCU8_TO_U16, dir,
                                                    strlen(dir)+1, &cvt))) {

            if (info.lpFile)
                free(info.lpFile);

            if (info.lpParameters)
                free(info.lpParameters);

            if (info.lpDirectory)
                free(info.lpDirectory);

            Dos9_ShowErrorMessage(DOS9_FAILED_ALLOCATION, "libcu8_xconvert()", FALSE);

            return -1;

        }

        for (chr = info.lpDirectory;*chr;chr ++) {
            if (*chr == L'/')
                *chr = L'\\';
        }

    }

    /* shellexecute seem to have trouble to handle forward slashes */
    for (chr = info.lpFile;*chr;chr ++) {
        if (*chr == L'/')
            *chr = L'\\';
    }



	ShellExecuteExW(&info);

	if (wait)
		WaitForSingleObject(info.hProcess, INFINITE);

	CloseHandle(info.hProcess);


    free(info.lpFile);
    free(info.lpParameters);
    free(info.lpDirectory);

	return 0;
}


#else
int Dos9_StartFile(const char* file, const char* args, const char* dir,
					int mode, int wait)
{
	SHELLEXECUTEINFO info;
	char buf[FILENAME_MAX],
         *chr;

	memset(&info, 0, sizeof(info));
	info.cbSize = sizeof(info);
	info.fMask =  SEE_MASK_NOASYNC |
        ((mode & START_MODE_BACKGROUND) ? (SEE_MASK_NO_CONSOLE) : (0));
	info.lpVerb = NULL;
	info.lpDirectory = dir;
	info.nShow = mode & ~START_MODE_BACKGROUND;

    snprintf(buf, sizeof(buf), "%s", file);



    /* apply Dos9 internal environment variables */
	Dos9_ApplyEnv(lpeEnv);

    /* shellexecute seem to have trouble to handle forward slashes */
    for (chr = buf;*chr;chr ++) {
        if (*chr == '/')
            *chr = '\\';
    }

    info.lpParameters = args;
    info.lpFile = buf;

	ShellExecuteExA(&info);

	if (wait)
		WaitForSingleObject(info.hProcess, INFINITE);

	CloseHandle(info.hProcess);

	return 0;
}
#endif
#else /* !defined(WIN32)  */

#if defined(XDG_OPEN)
#define Dos9_StartFile(file, args, dir, mode, wait) \
           if (mode & START_MODE_BACKGROUND) { \
                Dos9_StartFile_S(file, args, dir, mode, wait); \
           } else { \
               Dos9_StartFile_X(file, args, dir, mode, wait); \
           }
#else
#define Dos9_StartFile(file, args, dir, mode, wait) \
           Dos9_StartFile_S(file, args, dir, mode, wait)
#endif /* XDG_OPEN */


#if defined(XDG_OPEN)
int Dos9_StartFile_X(const char* file, const char* args, const char* dir,
					int mode, int wait)
{
    pid_t pid;

    pid = fork();

    ESTR* tmp;
    char* arg[FILENAME_MAX];
    int status, i;

    if (pid == 0) {

        /* we are in the son */
        if (dir && chdir(dir) == -1) {
            Dos9_ShowErrorMessage(DOS9_DIRECTORY_ERROR | DOS9_PRINT_C_ERROR,
                                    dir,
                                    FALSE);
            exit(-1);
        }

        arg[0] = XDG_OPEN;
        arg[1] = file;

        i=2;

        while ((i < (FILENAME_MAX-1))
               && (args = Dos9_GetNextParameterEs(args, tmp=Dos9_EsInit()))) {
            arg[i++] = Dos9_EsToChar(tmp);
        }

        arg[i] = NULL;

        Dos9_OpenOutput(lppsStreamStack, "NULL", DOS9_STDOUT | DOS9_STDERR, 0);

        /* apply Dos9 internal environment variables */
        Dos9_ApplyEnv(lpeEnv);

        if (execvp(XDG_OPEN, arg) == -1) {
            Dos9_ShowErrorMessage(DOS9_COMMAND_ERROR | DOS9_PRINT_C_ERROR,
                                    XDG_OPEN,
                                    FALSE);
            exit(-1);
        }

    } else if (pid == -1) {

        Dos9_ShowErrorMessage(DOS9_FAILED_FORK | DOS9_PRINT_C_ERROR,
                        __FILE__ "/Dos9_StartFile()",
                        FALSE);
        return -1;

    } else {

        if (wait)
            waitpid(pid, &status, 0);

    }

    return 0;

}
#endif /* defined(XDG_OPEN) */

int Dos9_StartFile_S(const char* file, const char* args, const char* dir,
					int mode, int wait)
{
    pid_t pid;

    pid = fork();

    ESTR* tmp;
    char* arg[FILENAME_MAX];
    int status, i;

    if (pid == 0) {

        Dos9_OpenOutput(lppsStreamStack, "NUL", DOS9_STDOUT | DOS9_STDERR, 0);

        /* we are in the son */
        if (dir && chdir(dir) == -1)
            Dos9_ShowErrorMessage(DOS9_DIRECTORY_ERROR | DOS9_PRINT_C_ERROR,
                                    dir,
                                    1);

        arg[0] = file;
        i=1;

        while ((i < (FILENAME_MAX-1))
               && (args = Dos9_GetNextParameterEs(args, tmp=Dos9_EsInit()))) {
            arg[i++] = Dos9_EsToChar(tmp);
        }

        arg[i] = NULL;

        /* apply Dos9 internal environment variables */
        Dos9_ApplyEnv(lpeEnv);

        if (execvp(file, arg) == -1)
            Dos9_ShowErrorMessage(DOS9_COMMAND_ERROR | DOS9_PRINT_C_ERROR,
                                    file,
                                    1);

    } else if (pid == -1) {

        Dos9_ShowErrorMessage(DOS9_FAILED_FORK | DOS9_PRINT_C_ERROR,
                        __FILE__ "/Dos9_StartFile()",
                        FALSE);
        return -1;

    } else {

        if (wait)
            waitpid(pid, &status, 0);

    }

    return 0;

}

#endif // defined

void Dos9_UseBackSlash(char* line)
{
	while (line = strchr(line, '/')) *(line ++) = '\\';

}

int Dos9_CmdStart(char* line)
{
	ESTR *param = Dos9_EsInit(),
         *tmp;

	char wait = FALSE,
		 mode = START_MODE_NONE;

	char dirbuf[FILENAME_MAX],
		 file[FILENAME_MAX],
		 *dir = NULL;

	line += 5;

	while (line = Dos9_GetNextParameterEs(line, param)) {

		if (!stricmp("/wait", Dos9_EsToChar(param))) {

			wait = TRUE;

		} else if (!stricmp("/min", Dos9_EsToChar(param))) {

			mode = START_MODE_MIN | (mode & START_MODE_BACKGROUND);

		} else if (!stricmp("/max", Dos9_EsToChar(param))) {

			mode = START_MODE_MAX | (mode & START_MODE_BACKGROUND);

		} else if (!stricmp("/d", Dos9_EsToChar(param))) {

			if (((line = Dos9_GetNextParameterEs(line, param)) == NULL)
				|| dir != NULL) {

				Dos9_ShowErrorMessage(DOS9_UNEXPECTED_ELEMENT, "/d", NULL);
				goto error;

			}

			strncpy(dirbuf, Dos9_EsToChar(param), sizeof(dirbuf));
			dirbuf[FILENAME_MAX-1]='\0';

			dir = dirbuf;

		} else if (!stricmp("/b", Dos9_EsToChar(param))) {

		    mode |= START_MODE_BACKGROUND;

		} else if (!stricmp("/normal", Dos9_EsToChar(param))) {

		    /* Not implemented yet (shall we do so ? seems that it
               does not affect anything anyway ...) */

		} else {

			break;

		}

	}

	if (line == NULL) {

		Dos9_ShowErrorMessage(DOS9_EXPECTED_MORE, "START", NULL);
		goto error;

	}

	strncpy(file, Dos9_EsToChar(param), sizeof(file));
	file[FILENAME_MAX-1]='\0';

    Dos9_GetEndOfLine(line, param);

    if (*file == '\0') {

        /* If the file specified was "", make the command line somehow
           equivalent to :

           start [switches] "%comspec%" /C "param"

         */

        Dos9_GetExeFilename(file, FILENAME_MAX);
        tmp = Dos9_EsInit();

        Dos9_EsCat(tmp, "/C \"");
        Dos9_EsCat(tmp, param->str);
        Dos9_EsCat(tmp, "\"");

        Dos9_EsFree(param);
        param = tmp;
    }

#ifdef WIN32
	Dos9_UseBackSlash(line);
#endif // WIN32

	if (Dos9_StartFile(file,
						param->str,
						dir,
						mode,
						wait))
		goto error;

	Dos9_EsFree(param);
	return 0;

	error:

		Dos9_EsFree(param);
		return -1;

}
