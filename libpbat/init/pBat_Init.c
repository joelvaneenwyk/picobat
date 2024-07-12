/*
 *
 *   libpBat - The pBat project
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
#include "../libpBat.h"
#include "../libpBat-int.h"

#include "../../config.h"

#ifdef WIN32
  #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
  #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x4
  #endif
#endif

LIBPBAT int pBat_LibInit(void)
{
    #if defined(WIN32) && defined(LIBPBAT_W10_ANSI)
    /* Enable VT processing. */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;

    GetConsoleMode(hOut, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, mode);
    #endif

    #if defined(WIN32) && defined(PBAT_USE_LIBCU8) && PBAT_USE_LIBCU8==1
    if (libcu8_init(NULL) ==  -1)
        return -1;
    #endif // defined

    return 0;
}

LIBPBAT void pBat_LibClose(void)
{
}
