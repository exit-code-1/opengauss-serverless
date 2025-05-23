#-------------------------------------------------------------------------
#
# Makefile for ecpg library
#
# Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
# Portions Copyright (c) 1994, Regents of the University of California
#
# src/common/interfaces/ecpg/ecpglib/Makefile
#
#-------------------------------------------------------------------------

subdir = src/common/interfaces/ecpg/ecpglib
top_builddir = ../../../../..
include $(top_builddir)/src/Makefile.global

NAME= ecpg
SO_MAJOR_VERSION= 6
SO_MINOR_VERSION= 4

override CPPFLAGS := -I../include -I$(top_srcdir)/src/common/interfaces/ecpg/include \
	-I$(libpq_srcdir) -I$(top_builddir)/src/common/port -DFRONTEND $(CPPFLAGS)
override CFLAGS += $(PTHREAD_CFLAGS)

override CPPFLAGS := $(filter-out -fPIE, $(CPPFLAGS)) -fPIC
override CFLAGS := $(filter-out -fPIE, $(CFLAGS)) -fPIC

# Need to recompile any libpgport object files
LIBS := $(filter-out -lpgport, $(LIBS))

ifneq "$(MAKECMDGOALS)" "clean"
  ifneq "$(MAKECMDGOALS)" "distclean"
    ifneq "$(shell which g++ |grep hutaf_llt |wc -l)" "1"
      -include $(DEPEND)
    endif
  endif
endif
OBJS= execute.o typename.o descriptor.o sqlda.o data.o error.o prepare.o memory.o \
	connect.o misc.o path.o pgstrcasecmp.o \
	$(filter snprintf.o strlcpy.o win32setlocale.o isinf.o, $(LIBOBJS))

# thread.c is needed only for non-WIN32 implementation of path.c
ifneq ($(PORTNAME), win32)
OBJS += thread.o
endif

SHLIB_LINK = -L../pgtypeslib -lpgtypes $(libpq) $(filter -lintl -lm, $(LIBS)) $(PTHREAD_LIBS)
SHLIB_PREREQS = submake-libpq submake-pgtypeslib

SHLIB_EXPORTS = exports.txt

ifeq ($(PORTNAME), win32)
# Link to shfolder.dll instead of shell32.dll
SHLIB_LINK += -lshfolder
endif

all: all-lib

.PHONY: submake-pgtypeslib
submake-pgtypeslib:
	$(MAKE) -C $(top_builddir)/src/common/interfaces/ecpg/pgtypeslib all

# Shared library stuff
include $(top_srcdir)/src/Makefile.shlib

# We use some port modules verbatim, but since we need to
# compile with appropriate options to build a shared lib, we can't
# necessarily use the same object files as the backend uses. Instead,
# symlink the source files in here and build our own object file.

path.cpp pgstrcasecmp.cpp snprintf.cpp strlcpy.cpp thread.cpp win32setlocale.cpp isinf.cpp: % : $(top_srcdir)/src/common/port/%
	rm -f $@ && $(LN_S) $< .

misc.o: misc.cpp $(top_builddir)/src/common/port/pg_config_paths.h
path.o: path.cpp $(top_builddir)/src/common/port/pg_config_paths.h

$(top_builddir)/src/common/port/pg_config_paths.h:
	$(MAKE) -C $(top_builddir)/src/common/port pg_config_paths.h

install: all installdirs install-lib

installdirs: installdirs-lib

uninstall: uninstall-lib

clean distclean: clean-lib
	rm -f $(OBJS) *.depend
	rm -f path.cpp pgstrcasecmp.cpp snprintf.cpp strlcpy.cpp thread.cpp win32setlocale.cpp

maintainer-clean: distclean maintainer-clean-lib
