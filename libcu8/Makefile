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

SRCDIR = .
ifneq (,$(wildcard $(SRCDIR)/femto-config.mk))
    include $(SRCDIR)/femto-config.mk
endif

CFLAGS_LIB_CU8 += $(LIBCU8_INC) -I./include

SRC_FILES_CU8 = src/init.c src/encoding.c src/read.c src/write.c \
			src/fdio.c src/spawn.c src/stat.c src/dup.c src/find.c src/dir.c \
			src/dirent.c src/iconv/iconv.c

OBJ_FILES_CU8 = $(SRC_FILES_CU8:.c=.o)

all: libcu8.a

%.o: %.c config
	$(CC) $(CFLAGS_LIB_CU8) -c -o $@ $<

clean:
	rm -f $(OBJ_FILES_CU8) libcu8.a
	$(MAKE) -C test clean

test:
	$(MAKE) -C test run-tests

libcu8.a: $(OBJ_FILES_CU8)
	$(AR) r $@ $(OBJ_FILES_CU8)
	$(RANLIB) $@

bin:

config:
	$(MAKE) -C ../ config

# stuff to check
PROGRAMS =
LIBS = iconv
FUNCTIONS = _findfirst _findfirst32 _findfirst64 _findfirst32i64 \
			_findfirst64i32 _stat _stat32 _stat64 _stat64i32 \
			_stat32i64
OPTIONS =
DEFAULTOPTIONS =
SUBCONFIG =
ADDITIONALVARS =

.PHONY: all bin clean $(SUBDIRS) $(SUBDIRS_CLEAN) po-merge test
