# Femto - FEmto's a Makefile TOol
# Copyright (C) 2018 Romain GARBI
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished  to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

SRCDIR ?= .

ifneq (,$(wildcard $(SRCDIR)/femto-config.mk))
	include $(SRCDIR)/femto-config.mk
endif

USEOPTIONS =$(addprefix use-,$(OPTIONS))
USEOPTIONSX = $(addsuffix .no-gen,$(USEOPTIONS))
NOOPTIONS =$(addprefix no-,$(OPTIONS))
NOOPTIONSX = $(addsuffix .no-gen,$(NOOPTIONS))
DEFAULTOPTIONSX = $(addsuffix .no-gen,$(DEFAULTOPTIONS))
SUBCONF =$(addsuffix .config,$(SUBCONFIG))
SUBCLEAN =$(addsuffix .femto-clean,$(SUBCONFIG))
CONFIGVARS =                                 \
	$(addprefix prg_,$(PROGRAMS))            \
	$(addprefix path_,$(PROGRAMS))           \
	$(addprefix fn_,$(FUNCTIONS))            \
	$(addprefix lib_,$(LIBS))                \
	$(addprefix use_,$(OPTIONS))             \
	$(ADDITIONALVARS)

$(PROGRAMS):
	@echo "Looking for program : $@ ..."
	@if which $@ 2> /dev/null; then \
		echo prg_$@=1 >> femto-config.mk; \
		echo path_$@=`which $@` >> femto-config.mk; \
		echo "	found"; \
	else \
		echo "	none"; \
		echo prg_$@=0 >> femto-config.mk; \
		echo path_$@=0 >> femto-config.mk; \
	fi;

$(LIBS):
	@echo "Looking for lib : $@ ..."
	@echo "int main() { return 0;}" > config.c
	@if $(CC) -o femto-test.out config.c $(CFLAGS) $(LDFLAGS) -l$@ -O0 -s 2> /dev/null;	then \
		echo "lib_$@ = 1" >> femto-config.mk; \
		echo "	found"; \
	else \
		echo "	none"; \
		echo "lib_$@ = 0" >> femto-config.mk; \
	fi;

$(FUNCTIONS):
	@echo "Looking for : $@ ..."
	@sed -e 's,[@]fn[@],$@,g' -e 's,[@]fnp[@],$(shell echo $@ | sed -e 's,/,_,g' -e 's,[.],_,g'),g'< config.c.in > config.c
	@if $(CC) -o femto-test.out config.c $(CFLAGS) $(LDFLAGS) -O0 -s 2> /dev/null; then \
		echo "fn_$@ = 1" >> femto-config.mk; \
		echo "	found"; \
	else \
		echo "fn_$@ = 0" >> femto-config.mk; \
		echo "	none"; \
	fi;

$(FLAGS):
	@echo "Looking for flag $@ ..."
	@sed -e 's,[@]fn[@],$@,g' -e 's,[@]fnp[@],$(shell echo $@ | sed -e 's,/,_,g' -e 's,[.],_,g'),g'< config.c.in > config.c
	@if $(CC) -o femto-test.out config.c $(CFLAGS) $(LDFLAGS) -f$@ -O0 -s 2> /dev/null; then \
		echo "flag_$@ = 1" >> femto-config.mk; \
		echo "	supported"; \
	else \
		echo "flag_$@ = 0" >> femto-config.mk; \
		echo "	not supported"; \
	fi;

$(USEOPTIONS):
	@echo "$(subst use-,use_,$@)=1" >> femto-config.mk
	$(MAKE) config.h

$(NOOPTIONS):
	@echo "$(subst no-,use_,$@)=0" >> femto-config.mk
	$(MAKE) config.h

$(USEOPTIONSX):
	@echo "$(subst use-,use_,$(basename $@))=1" >> femto-config.mk

$(NOOPTIONSX):
	@echo "$(subst no-,use_,$(basename $@))=0" >> femto-config.mk

localmk:
	if [ ! -f femto-subst ]; then \
		echo Clearing femto-config.mk; \
		echo "" > femto-config.mk; \
	fi

$(SUBCONF):
	$(MAKE) -C $(basename $@) config

config: localmk $(PROGRAMS) $(LIBS) $(FUNCTIONS) $(FLAGS) $(DEFAULTOPTIONSX) $(SUBCONF)
	$(MAKE) config.h

%.h: %.h.in femto-subst
	./femto-subst < $< > $@

$(SUBCLEAN):
	$(MAKE) -C $(basename $@) femto-clean

femto-clean: $(SUBCLEAN)
	rm -f femto-test.out femto-subst femto-config.mk config.h config.c

femto-subst:
	if [ ! -f femto-subst ]; then \
		echo '#!/bin/sh' > femto-subst; \
		echo "sed $(foreach v,$(CONFIGVARS),-e 's,[@]$(v)[@],$($(v)),g')" >> femto-subst; \
		chmod +x femto-subst; \
	fi;

.PHONY: config femto-clean localmk $(SUBCONF) $(FUNCTIONS) $(PROGRAMS) $(LIBS) \
	$(FLAGS) $(NOOPTIONS) $(NOOPTIONSX) $(USEOPTIONS) $(USEOPTIONSX) femto-subst
