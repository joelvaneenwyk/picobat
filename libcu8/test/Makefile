# Copyright (c) 2014, 2015, 2016 Romain GARBI
#
# All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
# * Neither the name of the name of Romain Garbi (Darkbatcher) nor the
#   names of its contributors may be used to endorse or promote products
#   derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY ROMAIN GARBI AND CONTRIBUTORS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL ROMAIN GARBI AND CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES
# LOSS OF USE, DATA, OR PROFITS OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

ROOTDIR = ..
ifneq (,$(wildcard $(ROOTDIR)/femto-config.mk))
    include $(ROOTDIR)/femto-config.mk
endif

ifeq ($(fn_WIN32),1)
	LDFLAGS_PBAT += -lshlwapi
endif

noinst_SOURCES = convert.c ansi.c invalid.c read.c read850.c readconsole.c fgetc850.c writeconsole.c  \
fgets_oversize.c fgets_nl.c
noinst_PROGRAMS = $(noinst_SOURCES:.c=$(EXEC_SUFFIX))

LDADD_CU8_TEST += -L$(ROOTDIR) -lcu8
CFLAGS_CU8_TEST += -I$(ROOTDIR)/include
LDFLAGS_CU8_TEST += $(LDFLAGS)

all: $(noinst_PROGRAMS)

%: %.c libcu8
	$(CC) -o $@ $< $(LDADD_CU8_TEST) $(CFLAGS_CU8_TEST) $(LDFLAGS_CU8_TEST)

libcu8:
	$(MAKE) -C $(ROOTDIR)

clean:
	rm -f *.o $(noinst_PROGRAMS)

# Run the tests
run-tests: all
	@for i in $(noinst_PROGRAMS); do \
		echo -n "Running test '$$i'... " ; \
		./$$i ; \
		case $$? in \
			0) echo  "Ok";; \
			*) echo "Failed"; \
			echo ERROR: `[ -f log_msg ] && cat log_msg`; \
			exit -1;; \
		esac;\
	done; 2>&1

.PHONY: run-tests all clean
.SUFFIXES: .c .exe
