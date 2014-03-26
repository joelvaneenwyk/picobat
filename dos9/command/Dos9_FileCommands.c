/*
 *
 *   Dos9 - A Free, Cross-platform command prompt - The Dos9 project
 *   Copyright (C) 2010-2014 DarkBatcher
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
#include <ctype.h>
#include <time.h>

#include <libDos9.h>

#ifdef _POSIX_C_SOURCE

    #define Dos9_Mkdir(name, mode) mkdir(name, mode)

#elif defined WIN32

    #define Dos9_Mkdir(name, mode) mkdir(name)

#endif /* defined linux */

#include "../lang/Dos9_Lang.h"
#include "../lang/Dos9_Help.h"
#include "../errors/Dos9_Errors.h"
#include "../core/Dos9_Core.h"
#include "Dos9_FileCommands.h"


int Dos9_CmdDel(char* lpLine)
{
    char *lpNextToken, *lpToken;
    ESTR* lpEstr=Dos9_EsInit();
    char  bParam=0, cChar, bDel=TRUE;
    short wAttr=0;
    char  lpName[FILENAME_MAX]="\0";
    int iFlag=DOS9_SEARCH_DEFAULT;
    FILELIST* lpFileList, *lpSaveList;

    if (!(lpLine=strchr(lpLine, ' '))) {

        Dos9_ShowErrorMessage(DOS9_EXPECTED_MORE, "DEL / ERASE", FALSE);
        Dos9_EsFree(lpEstr);
        return -1;

    }

    while ((lpNextToken=Dos9_GetNextParameterEs(lpLine, lpEstr))) {

        lpToken=Dos9_EsToChar(lpEstr);

        if (!stricmp(lpToken, "/P")) {

            /* Demande une confirmation avant la suppression */

            bParam|=DOS9_ASK_CONFIRMATION;

        } else if (!stricmp(lpToken, "/F")) {

            /* supprime les fichiers en lecture seule */

            bParam|=DOS9_DELETE_READONLY;

        } else if (!stricmp(lpToken, "/S")) {

            /* proc�de r�cursivement */

            iFlag|=DOS9_SEARCH_RECURSIVE;

        } else if (!stricmp(lpToken, "/Q")) {

            /* pas de confirmation pour suppression avec caract�res g�n�riques */

            bParam|=DOS9_DONT_ASK_GENERIC;

        } else if (!strnicmp(lpToken, "/A", 2)) {

            /* gestion des attributs */
            lpToken+=2;
            if (*lpToken==':') lpToken++;

            wAttr=Dos9_MakeFileAttributes(lpToken);

            iFlag^=DOS9_SEARCH_NO_STAT;

        } else if (!strcmp("/?", lpToken)) {

            Dos9_ShowInternalHelp(DOS9_HELP_DEL);

            Dos9_EsFree(lpEstr);
            return -1;

        } else {

            if (*lpName != '\0') {

                /* si un nom a �t� donn�, on affiche, l'erreur */

                Dos9_ShowErrorMessage(DOS9_UNEXPECTED_ELEMENT, lpToken, FALSE);
                Dos9_EsFree(lpEstr);
                return -1;

            }

            strncpy(lpName, lpToken, FILENAME_MAX);
        }

        lpLine=lpNextToken;

    }

    if (*lpName== '\0') { /* si aucun nom n'a �t� donn� */

        Dos9_ShowErrorMessage(DOS9_EXPECTED_MORE, "DEL / ERASE", FALSE);
        Dos9_EsFree(lpEstr);
        return -1;

    }

    if (!(bParam & DOS9_DELETE_READONLY)) {

        /* par d�faut, la recherche ne prend pas en compte la recherche des fichiers en lecture seule */
        wAttr|=DOS9_CMD_ATTR_READONLY_N | DOS9_CMD_ATTR_READONLY;

    }

    wAttr|=DOS9_CMD_ATTR_DIR | DOS9_CMD_ATTR_DIR_N; /* ne s�lectionne que le fichiers */

    if ((Dos9_StrToken(lpName, '*') || Dos9_StrToken(lpName, '?')) && !(bParam & DOS9_DONT_ASK_GENERIC)) { // si la recherche est g�n�rique on met la demande
        bParam|=DOS9_ASK_CONFIRMATION;                                      // de confirmation sauf si `/Q` a �t� sp�cifi�
    }

    printf("Looking for `%s'\n", lpName);

    if ((lpFileList=Dos9_GetMatchFileList(lpName, iFlag))) {
        lpSaveList=lpFileList;
        while (lpFileList) {

            /* on demande confirmation si la demande de confirmation est active */
            if (bParam & DOS9_ASK_CONFIRMATION) {
                cChar=0;
                while (toupper(cChar)!='Y' && toupper(cChar)!='N' && toupper(cChar)!='A')
                {
                    printf("<DEL> Voulez vous suprimmer `%s' ? (y/n/a) ", lpFileList->lpFileName);
                    cChar=getchar();
                    while (getchar()!='\n') {};
                }

                if (toupper(cChar)=='N') {
                    /* si l'utilisateur refuse la suppression du fichier */
                    lpFileList=lpFileList->lpflNext;
                    continue;
                } else if (toupper(cChar)=='A') {
                    /* si l'utilisateur autorise tous les fichiers */
                    bParam&=~DOS9_ASK_CONFIRMATION;
                }
            }

            /* on v�rifie que le fichier poss�de les bons attributs pour la suppression */
            if ((Dos9_CheckFileAttributes(wAttr, lpFileList))) {
                printf("<DEBUG> supression de `%s'\n", lpFileList->lpFileName);
            } else {
                printf("<DEBUG> Fichier `%s' non suprimm� (attributs incorrects)\n", lpFileList->lpFileName);
            }

            /* on passe au fichier suivant */
            lpFileList=lpFileList->lpflNext;
        }

        Dos9_FreeFileList(lpSaveList);
    } else {

        printf("Aucun fichier `%s' n'a ete trouve\n", lpName);

    }

    Dos9_EsFree(lpEstr);
    return 0;
}

int iDirNb,
    iFileNb;
short wAttr;
char bSimple;

void Dos9_CmdDirShow(FILELIST* lpElement)
{
    char lpType[]="D RHSA ",
         lpSize[16];

    struct tm* lTime;

    /* This structures get only one argument at a time,
       thus ``lpElement->lpNext'' is inconsistent */

    if (Dos9_CheckFileAttributes(wAttr, lpElement)) {

        /* if the file has right attributes */

        if (!bSimple) {

            strcpy(lpType, "       ");
            if (Dos9_GetFileMode(lpElement) & DOS9_FILE_DIR) {
                lpType[0]='D';
                iDirNb++;

            } else {

                iFileNb++;

            }

            if (Dos9_GetFileMode(lpElement) & DOS9_FILE_READONLY) lpType[2]='R';

            if (Dos9_GetFileMode(lpElement) & DOS9_FILE_HIDDEN) lpType[3]='H';

            if (Dos9_GetFileMode(lpElement) & DOS9_FILE_SYSTEM) lpType[4]='S';

            if (Dos9_GetFileMode(lpElement) & DOS9_FILE_ARCHIVE) lpType[5]='A';

            /* !! Recyclage de la variable lpFilename pour afficher la taille du fichier */
            Dos9_FormatFileSize(lpSize, 16, Dos9_GetFileSize(lpElement));

            lTime=localtime(&Dos9_GetModifTime(lpElement));

            printf("%02d/%02d/%02d %02d:%02d\t%s\t%s\t%s\n",lTime->tm_mday , lTime->tm_mon+1, 1900+lTime->tm_year, lTime->tm_hour, lTime->tm_min, lpSize, lpType, lpElement->lpFileName);

        } else {

            printf("%s\n", lpElement->lpFileName);

        }
    }
}

int Dos9_CmdDir(char* lpLine)
{
    char *lpNext,
         *lpToken,
         lpFileName[FILENAME_MAX]={0};

    int iFlag=DOS9_SEARCH_DEFAULT | DOS9_SEARCH_DIR_MODE;

    ESTR* lpParam=Dos9_EsInit();

     //if (lpNext=strchr(lpLine, ' ')) *lpNext='\0';
    lpNext=lpLine+3;

    wAttr=DOS9_CMD_ATTR_ALL;
    bSimple=FALSE;

    while ((lpNext=Dos9_GetNextParameterEs(lpNext, lpParam))) {

        lpToken=Dos9_EsToChar(lpParam);

        if (!strcmp(lpToken, "/?")) {

            Dos9_ShowInternalHelp(DOS9_HELP_DIR);
            return 0;

        } else if (!stricmp("/b", lpToken)) {

            /* use the simple dir output */
            bSimple=TRUE;
            if (!wAttr) iFlag|=DOS9_SEARCH_NO_STAT;
            iFlag|=DOS9_SEARCH_NO_PSEUDO_DIR;

        } else if (!stricmp("/s", lpToken)) {

            /* set the command to recusive */
            iFlag|=DOS9_SEARCH_RECURSIVE;

        } else if (!strnicmp("/a", lpToken,2)) {

            /* uses the attributes command */
            lpToken+=2;
            if (*lpToken==':') lpToken++;
            wAttr=Dos9_MakeFileAttributes(lpToken);
            iFlag&=~DOS9_SEARCH_NO_STAT;

        } else {

            if (*lpFileName) {
                   Dos9_ShowErrorMessage(DOS9_UNEXPECTED_ELEMENT, lpToken, FALSE);
                   Dos9_EsFree(lpParam);
                   return -1;
            }
            strncpy(lpFileName, lpToken, FILENAME_MAX);

        }
    }

    if (!*lpFileName) {
        /* if no file or directory name have been specified
           the put a correct value on it */

        *lpFileName='*';
        lpFileName[1]='\0';

    }

    /* do a little global variable setup before
       starting file research */
    iDirNb=0;
    iFileNb=0;

    if (!bSimple) puts(lpDirListTitle);

    /* Get a list of file and directories matching to the
       current filename and options set */
    if (!(Dos9_GetMatchFileCallback(lpFileName, iFlag, Dos9_CmdDirShow))
        && !bSimple)
            puts(lpDirNoFileFound);

    if (!bSimple) printf(lpDirFileDirNb, iFileNb, iDirNb);

    Dos9_EsFree(lpParam);

    return 0;
}


int Dos9_CmdMove(char* lpLine)
{
    return 0;
}

int Dos9_CmdCopy(char* lpLine)
{
    return 0;
}

int Dos9_CmdRen(char* lpLine)
{
    ESTR* lpEstr=Dos9_EsInit();
    char lpFileName[FILENAME_MAX]={0}, lpFileDest[FILENAME_MAX]={0};
    char* lpToken;

    if (!(lpLine=strchr(lpLine, ' '))) {
        Dos9_ShowErrorMessage(DOS9_EXPECTED_MORE, "DEL / ERASE", FALSE);
        Dos9_EsFree(lpEstr);
        return -1;
    }

    if ((lpLine=Dos9_GetNextParameterEs(lpLine, lpEstr))) {

        strncpy(lpFileName, Dos9_EsToChar(lpEstr), FILENAME_MAX);
        lpFileName[FILENAME_MAX-1]='\0'; // can't assume that what was buffered is NULL-terminated
                                        // see the C-89,99,11 standards for further informations
        strcpy(lpFileDest, lpFileName);

        if ((lpLine=Dos9_GetNextParameterEs(lpLine, lpEstr))) {

            /* removing old filename */
            lpLine=strrchr(lpFileDest, '\\');
            lpToken=strrchr(lpFileDest, '/');

            if (lpToken>lpLine) {
                lpLine=lpToken;
            }

            if (lpLine) {
                lpLine++;
                *lpLine='\0';
            }

            /* cat with new name */
            strncat(lpFileDest, Dos9_EsToChar(lpEstr), FILENAME_MAX-strlen(lpFileDest));
            lpFileDest[FILENAME_MAX-1]='\0';// can't assume that what was buffered is NULL-terminated
                                            // see the C-89,99,11 standards for further informations
            if (!printf("<DEBUG> renaming `%s` to `%s`\n", lpFileName, lpFileDest))
            {
                Dos9_ShowErrorMessage(DOS9_UNABLE_RENAME, lpFileName, FALSE); // if renaming fails
                Dos9_EsFree(lpEstr);
                return -1;
            }
            return 0;
        }
    }

    Dos9_ShowErrorMessage(DOS9_EXPECTED_MORE, "REN / RENAME", FALSE);
    Dos9_EsFree(lpEstr);
    return -1;
}

int Dos9_CmdRmdir(char* lpLine)
{
    ESTR* lpEstr=Dos9_EsInit();

    if (!(lpLine=Dos9_GetNextParameterEs(lpLine, lpEstr))) {

        Dos9_ShowErrorMessage(DOS9_EXPECTED_MORE, "MD/MKDIR", FALSE);
        goto error;

    }

    if ((lpLine=Dos9_GetNextParameterEs(lpLine, lpEstr))) {
        if (!strcmp(Dos9_EsToChar(lpEstr), "/?")) {

            Dos9_ShowInternalHelp(DOS9_HELP_RD);

        } else if (!stricmp(Dos9_EsToChar(lpEstr),"/Q")) {
            // on affiche ta m�re blah
        } else {

            if (rmdir(Dos9_EsToChar(lpEstr))) {
                Dos9_ShowErrorMessage(DOS9_MKDIR_ERROR, Dos9_EsToChar(lpEstr), FALSE);
                goto error;
            }

        }
    }

    Dos9_EsFree(lpEstr);
    return 0;

    error:
        Dos9_EsFree(lpEstr);
        return -1;
}

int Dos9_CmdMkdir(char* lpLine)
{
    ESTR* lpEstr=Dos9_EsInit();

    if (!(lpLine=Dos9_GetNextParameterEs(lpLine, lpEstr))) {

        Dos9_ShowErrorMessage(DOS9_EXPECTED_MORE, "MD/MKDIR", FALSE);
        goto error;

    }

    if ((lpLine=Dos9_GetNextParameterEs(lpLine, lpEstr))) {
        if (!strcmp(Dos9_EsToChar(lpEstr), "/?")) {

            Dos9_ShowInternalHelp(DOS9_HELP_MD);

        } else {

            if (Dos9_Mkdir(Dos9_EsToChar(lpEstr), 0777)) {

                Dos9_ShowErrorMessage(DOS9_MKDIR_ERROR, Dos9_EsToChar(lpEstr), FALSE);
                goto error;

            }
        }
    }

    Dos9_EsFree(lpEstr);
    return 0;

    error:
        Dos9_EsFree(lpEstr);
        return -1;

}
