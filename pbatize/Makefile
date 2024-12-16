#
#   pBat Makefiles, The pBat project
#   Copyright (C) 2012-2018  Romain Garbi, Teddy Astie
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

ROOTDIR = ..
include $(ROOTDIR)/config.mk


CFLAGS_PBATIZE += $(CFLAGS) $(MICROGETTEXT_INC) $(LIBPBAT_INC) \
				$(LIBMATHEVAL_INC) $(LIBCU8_INC)

LDFLAGS_PBATIZE += $(LDFLAGS) $(MICROGETTEXT_LD) $(LIBPBAT_LD) \
				$(LIBMATHEVAL_LD) $(LIBCU8_LD) \
				$(LPTHREAD_LD) $(LM_LD) $(LDL_LD)

ifeq ($(OS),Windows_NT)
	LDFLAGS_PBATIZE += -lshlwapi
endif

SRC_FILES_PBATIZE = pbatize.c pairs.c
OBJ_FILES_PBATIZE := $(SRC_FILES_PBATIZE:.c=.o)

all: pbatize

%.o: %.c pbat
	$(CC) $(CFLAGS_PBATIZE) -c -o $@ $<

pbatize: $(OBJ_FILES_PBATIZE)
	$(CC) $(CFLAGS_PBATIZE) -o pbatize$(EXEC_SUFFIX) $(OBJ_FILES_PBATIZE) $(LDFLAGS_PBATIZE)

clean:
	rm -f pbatize$(EXEC_SUFFIX)

bin: pbatize
	cp pbatize$(EXEC_SUFFIX) $(BINDIR)/cmd/

pbat:
	$(MAKE) -C ../pbat

.PHONY: all clean pbat bin
