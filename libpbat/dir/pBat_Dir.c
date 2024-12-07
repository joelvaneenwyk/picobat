/*
 *
 *   libpBat - The pBat project
 *   Copyright (C) 2010-2017 Romain GARBI
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

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif

#include <ctype.h>
#include "../libpBat.h"
#include "../libpBat-int.h"
#include "../../config.h"

#ifndef WIN32
#include <strings.h>
#endif

#if !defined(PBAT_USE_LIBCU8) && PBAT_USE_LIBCU8==1 || PBAT_USE_LIBCU8!=1

struct match_args_t {
    int flags;
    FILELIST* files;
    void (*callback)(FILELIST*);
};

static int __inline__ pBat_IsRegExpTrivial(char* exp)
{
    if (strpbrk(exp, "*?")) {
        return 0;
    } else {
        return 1;
    }
}

/* Return the next level of the string pointed to by up, taking
   account of various parameter.

   if *jok is zero, then, the return value points to the end of
   a string constituted by at least (but maybe more than) one level of
   regular expression, but without any joker or question mark inside
   them.

   if *jok is non-zero, then the return value refers to the end of a
   string of exactly one expression level that contains at least one
   question mark or joker */
char* pBat_GetNextLevel(char* up, char* base, int* jok)
{
    int joker = 0;
    char* prev = NULL;

    *jok = 0;

#ifdef WIN32
    if (base == NULL
        && !strncmp(up, "\\\\?", 3)
        && (*(up + 3)  == '\\' ))
        up += 3;
#endif

    while (*up) {

        switch (*up) {

        case '*':
        case '?':
            joker = 1;
            if (prev)
                return prev;
            *jok = 1;
            break;

        case '/':
#ifdef WIN32
        case '\\':
#endif
            if (joker)
                return up;
            prev = up;

        }

        up++;
    }

    return NULL;
}

static FILELIST* pBat_AddMatch(char* name, FILELIST* files, struct match_args_t* arg)
{
    FILELIST *file,
             block;

    if (arg->callback) {

        snprintf(block.lpFileName, FILENAME_MAX, "%s", name);

        if (!(arg->flags & PBAT_SEARCH_NO_STAT)) {
            stat(name, &(block.stFileStats));

#ifdef WIN32

            /* Using windows, more detailled directory information
               can be obtained */
            block.stFileStats.st_mode=pBat_GetFileAttributes(name);

#endif // WIN32
        }

        arg->callback(&block);

        // Previously this was (((void*)files)+1);
        return ((files)+1);

    }

    if ((file = malloc(sizeof(FILELIST))) == NULL)
        return NULL;

    snprintf(file->lpFileName, FILENAME_MAX, "%s", name);


    /* fixme : doing something less time-consuming is
       probably possible */
    if (!(arg->flags & PBAT_SEARCH_NO_STAT)) {
        stat(name, &(file->stFileStats));

#ifdef WIN32

        /* Using windows, more detailled directory information
           can be obtained */
        file->stFileStats.st_mode=pBat_GetFileAttributes(name);

#endif // WIN32
    }

    if (files)
        files->lpflNext = file;

    file->lpflNext = NULL;
    file->lpflPrevious = files;

    /* Well, this turns out to produce heaps */
    return file;
}

static int /* inline */ pBat_EndWithDirectoryMark(const char *dir)
{
    char *c = NULL;

    while (*dir)
        c = dir++;

    return c ? PBAT_TEST_SEPARATOR(c) : 0;
}

/* Fixme : This function is quite a lot unefficient under windows,
   its perfomances can be enhanced by using native winapi functions for
   file search */
static FILELIST* pBat_GetMatch(char* restrict base, char* restrict up, struct match_args_t* arg)
{
    FILELIST *ret = arg->files, *tmp;

    ESTR* path = NULL;
    char *item, *cleanup = NULL, basetmp[]="x:";

    DIR* dir = NULL;
    struct dirent* ent;

    int joker = 0;

    /* if something has already been found ... */
    if ((ret != (FILELIST*)-1) && (ret != NULL)
        && (arg->flags & PBAT_SEARCH_GET_FIRST_MATCH))
        return ret;

    if (base == NULL) {
        /* This is somewhat the top-level instance of pBat_GetMatch ...*/

        if ((arg->flags &
                (PBAT_SEARCH_DIR_MODE | PBAT_SEARCH_RECURSIVE))
            && pBat_DirExists(up)) {
            /* This regular expression is trivial however either dir mode
               or recursive modes have been specified. Thus we need to
               search matching files _inside_ the directory */

            /* First, check that the name does not end with a slash, if it
               is the case, remove it */
            if (!(arg->flags & PBAT_SEARCH_DIR_MODE)
                && (arg->callback != NULL)) {
                /* If dir mode is not specified, add that match too */
                if ((tmp = pBat_AddMatch(up, ret, arg)) == NULL)
                    goto err;

                arg->files = tmp;
            }

            ret = pBat_GetMatch(up, "*", arg);

            if (!(arg->flags & PBAT_SEARCH_DIR_MODE)
                && arg->callback == NULL) {
                if ((tmp = pBat_AddMatch(up, ret, arg)) == NULL)
                    goto err;

                ret = tmp;
            }

            goto end;

        }

        if (pBat_DirExists(up) || pBat_FileExists(up)) {
            /* this regular expression is trivial (indeed 'up' is the only
               matching file or directory */

            if ((tmp = pBat_AddMatch(up, ret, arg)) == NULL)
                goto err;

            return tmp;

        } else if (pBat_IsRegExpTrivial(up)) {
            /* We did not find anything but it is trivial though so there
               is no matching file */
            return NULL;
        }

        /* As this is the top level instance of file match, it is wise
           to check that *up does not refer to an absolute path, and such
           path use quite a special syntax that may mess up with the
           delimiter searching functions. Indeed in both *nixes and
           Windows, absolute path may start with '/' which is indeed
           a valid path delimiter.

           Windows also provide a wide variety of other syntaxes to refer
           to the concept of device :

                - x:\ : Traditional device root
                - \\.\ : Path referring to special objects
                - \\?\ : Path referring to the current computer
                - \\unc-name\ : A unc path

          */
        if (PBAT_TEST_ROOT_PATH(up)) {

            /* For convenience, if a '/' is
               encountered, just set *base to be the empty string, as no file may
               bear an empty name anyway. */

            base = "/"; /* simple as hell */

            if (up && *up)
                up ++;

        } else if (PBAT_TEST_DRIVE_PATH(up)) {

            base = basetmp;
            basetmp[0] = *up;

            if (up && *up)
                up++;

            if (up && *up)
                up++;

            if (up && *up)
                up++;

        } else if (PBAT_TEST_UNC_PATH(up)) {

            /* not implemented */

        }


    }

    /* we will need this */
    path = pBat_EsInit();

    item = up;

    /* search for the next item in the search in up */
    if (up == NULL) {

       item = "*";

    } else if ((up = pBat_GetNextLevel(up, base, &joker))) {

      cleanup = up;
      *cleanup = '\0';
       up ++;

    }

    if (!joker) {

        /* Check if the item is trivial before really trying
           to recurse in base folder ... */
        if (base != NULL) {

            pBat_EsCpy(path, base);
            if (!pBat_EndWithDirectoryMark(path->str))
                pBat_EsCat(path, PBAT_DEF_DELIMITER);
            pBat_EsCat(path, item);

        } else {

            pBat_EsCpy(path, item);

        }



        if (pBat_FileExists(path->str)) {
            /* if path corresponds to a file (ie. not a dir), there is two
               possibilities (a) up is NULL and then the files matches sinces
               we already reached top level, or (b) up is not NULL and then
               the file cannot match and it is wise to stop search in this
               folder */
            if (up == NULL) {

                if ((tmp = pBat_AddMatch(path->str, ret, arg)) == NULL)
                    goto err;

                ret = tmp;

            }
        }

        if (pBat_DirExists(path->str)) {
            /* if path corresponds to a dir, always continue search. But path
               is not necessarily to be added to results... ! */

            if (up == NULL) {

                /* add the file and browse if recursive. If the returns uses
                   callback, do not forget to add match first. */
                if (arg->callback != NULL) {
                    if ((tmp = pBat_AddMatch(path->str, ret, arg)) == NULL)
                        goto err;

                    ret = tmp;
                }

                if (arg->flags & PBAT_SEARCH_RECURSIVE) {

                    arg->files = ret;
                    ret = pBat_GetMatch(path->str, up, arg);

                }

                if (arg->callback == NULL) {
                    if ((tmp = pBat_AddMatch(path->str, ret, arg)) == NULL)
                        goto err;

                    ret = tmp;
                }

            } else {

                /* always browse */
                arg->files = ret;
                ret = pBat_GetMatch(path->str, up, arg);

            }

        }

        goto end;

    }

    /* Now we have checked every possible trivial dir, browse dir */
    if ((dir = opendir((base != NULL) ? (base) : ("."))) == NULL)
        goto end;

    /* loop through the directory entities */
    while ((ent = readdir(dir))) {

        /* skip basic pseudo dirs */
        if ((arg->flags & PBAT_SEARCH_NO_PSEUDO_DIR)
            && (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")))
            continue;

        /* if the file name definitely matches the expression */
        if (pBat_RegExpCaseMatch(item, ent->d_name)) {

            /* Compute the path of the current matching entity */
            if (base != NULL) {
                pBat_EsCpy(path, base);
                if (!pBat_EndWithDirectoryMark(path->str))
                    pBat_EsCat(path, PBAT_DEF_DELIMITER);
                pBat_EsCat(path, ent->d_name);
            } else {
                pBat_EsCpy(path, ent->d_name);
            }

            if (pBat_FileExists(path->str)) {
                /* The entity is a file, so only add if up is NULL */

                if (up == NULL) {

                    if ((tmp = pBat_AddMatch(path->str, ret, arg)) == NULL)
                        goto err;

                    ret = tmp;

                    if (arg->flags & PBAT_SEARCH_GET_FIRST_MATCH)
                            goto end;

                }

            } else {
                /* the entity is a directory, browse if (a) up is not NULL
                or (b) the search is recursive and up is NULL. Only add if
                up is NULL*/
                if (up == NULL) {

                    /* add the directory and browse if recursive. If the returns uses
                       callback, do not forget to add match first. */
                    if ((arg->callback != NULL)) {
                        if ((tmp = pBat_AddMatch(path->str, ret, arg)) == NULL)
                            goto err;

                        ret = tmp;

                        /* If the user only requested the first match */
                        if (arg->flags & PBAT_SEARCH_GET_FIRST_MATCH)
                            goto end;
                    }

                    if ((arg->flags & PBAT_SEARCH_RECURSIVE)
                        && (strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))) {

                        arg->files = ret;
                        ret = pBat_GetMatch(path->str, item, arg);

                        if ((ret == (FILELIST*)-1)
                            || ((ret != (FILELIST*)-1) && (ret != NULL)
                                && (arg->flags & PBAT_SEARCH_GET_FIRST_MATCH)))
                            goto end;

                    }

                    if (arg->callback == NULL) {
                        if ((tmp = pBat_AddMatch(path->str, ret, arg)) == NULL)
                            goto err;

                        ret = tmp;

                        /* If the user only requested the first match */
                        if (arg->flags & PBAT_SEARCH_GET_FIRST_MATCH)
                            goto end;
                    }

                } else {

                    /* always browse */
                    arg->files = ret;
                    ret = pBat_GetMatch(path->str, up, arg);

                    /* exit on error or if the user only requested the first
                       match */
                    if ((ret == (FILELIST*)-1)
                        || ((ret != (FILELIST*)-1) && (ret != NULL)
                            && (arg->flags & PBAT_SEARCH_GET_FIRST_MATCH)))
                        goto end;

                }

            }

        } else if ((arg->flags & PBAT_SEARCH_RECURSIVE)
                    && up == NULL
                    && (strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))) {

            /* Compute the path of the current matching entity */
            if (base != NULL) {
                pBat_EsCpy(path, base);
                if (!pBat_EndWithDirectoryMark(path->str))
                    pBat_EsCat(path, PBAT_DEF_DELIMITER);
                pBat_EsCat(path, ent->d_name);
            } else {
                pBat_EsCpy(path, ent->d_name);
            }

            if (pBat_DirExists(path->str)) {

                /* if it does not match *but* we are in recursive mode, browse
                   anyway */
                arg->files = ret;
                ret = pBat_GetMatch(path->str, item, arg);

            }

        }

    }

end:
    if (dir)
        closedir(dir);

    if (cleanup)
        *cleanup = '/';

    if (path)
        pBat_EsFree(path);

    return ret;

err:
    if (dir)
        closedir(dir);

    if (ret)
        pBat_FreeFileList(ret);

    if (path)
        pBat_EsFree(path);

    return (void*)-1;
}


LIBPBAT LPFILELIST  pBat_GetMatchFileList(char* lpPathMatch, int iFlag)
{
    struct match_args_t args;
    FILELIST* file;

    args.callback = NULL;
    args.flags = iFlag;
    args.files =  NULL;

    file = pBat_GetMatch(NULL, lpPathMatch, &args);

    if (file == (FILELIST*)-1) {

        fprintf(stderr, "Fatal ERROR : file search\n");
        exit(-1);

    }

    /* rewind through files */
    if (file)
        while (file->lpflPrevious)
            file = file->lpflPrevious;

    return file;
}

LIBPBAT int pBat_GetMatchFileCallback(char* lpPathMatch, int iFlag, void(*pCallBack)(FILELIST*))
{
    struct match_args_t args;
    FILELIST* file;

    args.callback = pCallBack;
    args.flags = iFlag;
    args.files =  NULL;

    file = pBat_GetMatch(NULL, lpPathMatch, &args);

    if (file == (FILELIST*)-1) {

        fprintf(stderr, "Fatal ERROR : file search\n");
        exit(-1);

    }

    return (int)file;
}
#endif


LIBPBAT const char *pBat_SeekPattern(const char* restrict match, const char* restrict pattern, size_t len)
{
    /* const char* tok; */
    int   i = 0;

    //printf("Looking for pattern \"%s\"[%d] in\"%s\"\n", pattern, len, match);

    while (*match) {

        i = 0;

    //    printf("\tAt \"%s\"\n", match);

        while ((i < len) && (match[i] == pattern[i]))
            i ++;

        if (len == i) {
    //        printf("OK\n");
            return match;
        }
        match ++;
    }

    //printf("FAIL\n");
    return NULL;
}

LIBPBAT int pBat_EndWithPattern(const char* restrict match, const char* restrict pattern, size_t len)
{
    size_t size = strlen(match);

    //printf("Comparing \"%s\" and \"%s\" with len %d[%d]...\n", match, pattern, len, size);

    if (size < len)
        return 0;

    match += size - len;

    //printf("====\"%s\" and \"%s\"\n", match, pattern);

    return !strcmp(match, pattern);

}

LIBPBAT int pBat_RegExpMatch(const char* restrict regexp, const char* restrict match)
{
	char* next;
    size_t size;

    //printf("*** Comparing \"%s\" et \"%s\"\n", regexp, match);

    if (!(regexp && match)) return FALSE;

    while (*regexp && *match) {

        switch (*regexp) {

            case '?':
                break;

            case '*':
                while (*regexp == '*' || *regexp == '?')
                    regexp ++;

                if (*regexp == '\0')
                    return 1;

                if ((next = strpbrk(regexp, "*?"))) {

                    size = next - regexp;

                    if ((match = pBat_SeekPattern(match, regexp, size))) {

                        //printf("Match found\n");

                        regexp += size;
                        match += size;

                        //printf("Next : \"%s\" and \"%s\"\n", match, regexp);

                        continue;

                    } else {

                        //printf("Failed\n");

                        return 0;

                    }

                } else {

                    //printf("Checking final point\n");

                    size = strlen(regexp);
                    return pBat_EndWithPattern(match, regexp, size);

                }

                break;

            default:
                if (*regexp  != *match)
                    return 0;
        }

        regexp = pBat_GetNextChar(regexp);
        match = pBat_GetNextChar(match);

    }

    if (*match || *regexp)
        return 0;

    //printf ("*** RETURN : OK\n");

    return 1;

}

LIBPBAT char* pBat_SeekCasePattern(const char* restrict match, const char* restrict pattern, size_t len)
{
    int   i = 0;

    //printf("Looking for pattern \"%s\"[%d] in\"%s\"\n", pattern, len, match);

    while (*match) {

        i = 0;

    //    printf("\tAt \"%s\"\n", match);

        while ((i < len) && (toupper(match[i]) == toupper(pattern[i])))
            i ++;

        if (len == i) {
    //        printf("OK\n");
            return (char *)match;
        }
        match ++;
    }

    //printf("FAIL\n");
    return NULL;
}

LIBPBAT int pBat_EndWithCasePattern(const char* restrict match, const char* restrict pattern, size_t len)
{
    size_t size = strlen(match);

    //printf("Comparing \"%s\" and \"%s\" with len %d[%d]...\n", match, pattern, len, size);

    if (size < len)
        return 0;

    match += size - len;

    //printf("====\"%s\" and \"%s\"\n", match, pattern);

    return !stricmp(match, pattern);

}

LIBPBAT int pBat_RegExpCaseMatch(const char* restrict regexp, const char* restrict match)
{
	char* next;
    size_t size;

    //printf("*** Comparing \"%s\" et \"%s\"\n", regexp, match);

    if (!(regexp && match)) return FALSE;

    while (*regexp && *match) {

        switch (*regexp) {

            case '?':
                break;

            case '*':
                while (*regexp == '*' || *regexp == '?')
                    regexp ++;

                if (*regexp == '\0')
                    return 1;

                if ((next = strpbrk(regexp, "*?"))) {

                    size = next - regexp;

                    if ((match = pBat_SeekCasePattern(match, regexp, size))) {

                        //printf("Match found\n");

                        regexp += size;
                        match += size;

                        //printf("Next : \"%s\" and \"%s\"\n", match, regexp);

                        continue;

                    } else {

                        //printf("Failed\n");

                        return 0;

                    }

                } else {

                    //printf("Checking final point\n");

                    size = strlen(regexp);
                    return pBat_EndWithCasePattern(match, regexp, size);

                }

                break;

            default:
                if (toupper(*regexp)  != toupper(*match))
                    return 0;
        }

        regexp = pBat_GetNextChar(regexp);
        match = pBat_GetNextChar(match);

    }

    if (*match || *regexp)
        return 0;

    //printf ("*** RETURN : OK\n");

    return 1;

}

LIBPBAT int pBat_FormatFileSize(char* lpBuf, int iLength, unsigned int iFileSize)
{
	int i=0, iLastPart=0;

	char* lpUnit[]= {"o", "ko", "mo", "go"};
	while (iFileSize>=1000) {
		iLastPart=iFileSize % 1000;
		iFileSize/=1000;
		i++;
	}
	if (iFileSize>=100) return snprintf(lpBuf, iLength, " %d %s", iFileSize, lpUnit[i%4]);
	if (iFileSize>=10) return snprintf(lpBuf, iLength, "  %d %s", iFileSize, lpUnit[i%4]);
	else {
		iLastPart=iLastPart/10;
		return snprintf(lpBuf, iLength, "%d,%.2d %s", iFileSize, iLastPart , lpUnit[i%4]);
	}
}

LIBPBAT size_t pBat_GetStaticLength(const char* str)
{
    const char *ptr = str,
               *orig = str;

    while (*str) {

        if (
#ifdef WIN32
            *str == '\\' ||
#endif
            *str == '/') {

            ptr = str;

        } else if (*str == '*' || *str == '?') {

            break;

        }

        str ++;

    }

    return (size_t)(ptr-orig);
}

int _pBat_FreeFileList(LPFILELIST lpflFileList)
{
	LPFILELIST lpflNext;

	while (lpflFileList) {

		lpflNext=lpflFileList->lpflNext;
		free(lpflFileList);
        lpflFileList = lpflNext;

	}

	return 0;
}


LIBPBAT int pBat_FreeFileList(LPFILELIST lpflFileList)
{

	_pBat_FreeFileList(lpflFileList);

	return 0;

}

#if defined(WIN32) && defined(PBAT_USE_LIBCU8) && PBAT_USE_LIBCU8==1
#include <libcu8.h>

int pBat_GetFileAttributes(const char* file)
{
    wchar_t *wfile;
    int ret;
    size_t conv;

    if (!(wfile = (wchar_t*)libcu8_xconvert(LIBCU8_TO_U16, file,
                                        strlen(file)+1, &conv)))
        return -1;

    ret = GetFileAttributesW(wfile);

    free(wfile);

    return ret;
}
#elif defined(WIN32) && !defined(PBAT_USE_LIBCU8) && PBAT_USE_LIBCU8==1
int pBat_GetFileAttributes(const char* file)
{
    return GetFileAttributes(file);
}
#else
int pBat_GetFileAttributes(const char* file)
{
    return 0;
}
#endif // defined
