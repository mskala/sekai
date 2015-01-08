## Process this file with automake to produce Makefile.in

#
# Makefile source for MIDIxlat
# Copyright (C) 2015  Matthew Skala
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Matthew Skala
# http://ansuz.sooke.bc.ca/
# mskala@ansuz.sooke.bc.ca
#

############################################################################

# BASIC AUTOTOOLS STUFF

ACLOCAL_AMFLAGS = -I m4

mvp:=$(if $(VPATH),$(VPATH),.)

empty:=
space:=$(empty) $(empty)
comma:=,
percent=%

SHELL=$(if $(wildcard @KLEKNEV@),@KLEKNEV@,$(if \
  $(wildcard /bin/bash),/bin/bash,/bin/sh))

############################################################################

# SILENT BUILD BLINKENLIGHTS

TSU_V_REDIR=$(if $(AM_V_at),>& /dev/null)

if COND_COLOUR
  escchar:=$(strip $(shell echo x | tr 'x' '\033'))
  ECHO_ANSI=echo
  ansi_default=$(escchar)[0m
  ansi_red=$(escchar)[31m
  ansi_green=$(escchar)[32m
  ansi_yellow=$(escchar)[33m
  ansi_blue=$(escchar)[34m
  ansi_magenta=$(escchar)[35m
  ansi_cyan=$(escchar)[36m
  ansi_pink=$(escchar)[35;1m
  ansi_white=$(escchar)[37;1m
  ansi_cleareol=$(escchar)[K
  ansi_clearescaped=\033\[K
  ansi_home=$(escchar)[1G
  am__@andt@v_CC_0=@echo "$(ansi_default)$(ansi_cleareol)  CC    " $@;
  am__@andt@v_CCLD_0=@echo "$(ansi_default)$(ansi_cleareol)  CCLD  " $@;
  am__@andt@v_GEN_0=@echo "$(ansi_default)$(ansi_cleareol)  GEN   " $@;
else
  ECHO_ANSI=echo
  ansi_default=
  ansi_red=
  ansi_green=
  ansi_yellow=
  ansi_blue=
  ansi_magenta=
  ansi_cyan=
  ansi_pink=
  ansi_white=
  ansi_cleareol=
  ansi_clearescaped=
  ansi_home=
endif

TSU_V_CP =$(TSU_V_CP_$(V))
TSU_V_CP_=$(TSU_V_CP_$(AM_DEFAULT_VERBOSITY))
TSU_V_CP_0=@$(ECHO_ANSI) \
  "$(ansi_cleareol)  $(ansi_default)CP$(ansi_default)    " $@;

TSU_V_TEXL=$(TSU_V_TEXL_$(V))
TSU_V_TEXL_=$(TSU_V_TEXL_$(AM_DEFAULT_VERBOSITY))
TSU_V_TEXL_0=$(ECHO_ANSI) \
  "$(ansi_cleareol)  $(ansi_green)TEX$(ansi_default)   " "$@($$I)";

############################################################################

# FILENAMES FOR AUTOTOOLS

bin_PROGRAMS = threev

threev_SOURCES = threev.c lk204.c lk204.h

############################################################################

# MAKE CLEAN

MOSTLYCLEANFILES =

CLEANFILES =

DISTCLEANFILES =

MAINTAINERCLEANFILES =

############################################################################

# AUTOMAKE'S RULES WILL GO HERE

automake_rules = here

############################################################################
