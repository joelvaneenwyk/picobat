/*
 *
 *   pBat - A Free, Cross-platform command prompt - The pBat project
 *   Copyright (C) 2010-2024 Romain GARBI
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
#if !defined(WIN32) && !defined(_X_OPEN_SOURCE)
#define _XOPEN_SOURCE 700
#endif

#include <stdio.h>
#include <string.h>

#ifndef WIN32
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "../../config.h"
#if PBAT_USE_LIBCU8==1
#include <libcu8.h>
#endif

#include "pBat_Core.h"
#include "../init/pBat_Init.h"

//#define PBAT_DBG_MODE
#include "pBat_Debug.h"

#include "../errors/pBat_Errors.h"
#include "../lang/pBat_Lang.h"


int pBat_Pipe(FILE** pipef)
{
    int pipedes[2],
        status = 0;

    /* init the receiving array */
    pipef[0] = pipef[1] = NULL;

    if (_pBat_Pipe(pipedes, 4096, O_BINARY) == -1)
        status  = -1;
    else if ((pipef[0] = fdopen(pipedes[0], "rb")) == NULL){

        close(pipedes[1]);
        close(pipedes[0]);
        status = -1;

    } else if ((pipef[1] = fdopen(pipedes[1], "wb")) == NULL) {
        fclose(pipef[0]);
        close(pipedes[1]);
        status = -1;
    } else {

        pBat_SetFdInheritance(pipedes[0], 0);
        pBat_SetFdInheritance(pipedes[1], 0);

    }

    return status;

}
