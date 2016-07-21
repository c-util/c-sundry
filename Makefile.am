# ------------------------------------------------------------------------------
# autotools

SHELL = bash
ACLOCAL_AMFLAGS = -I build/m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory
AUTOMAKE_OPTIONS = color-tests parallel-tests

GCC_COLORS ?= 'ooh, shiny!'
export GCC_COLORS

# remove targets if the command fails
.DELETE_ON_ERROR:

# keep intermediate files
.SECONDARY:

# keep the test-suite.log and Makefile around at all times
.PRECIOUS: $(TEST_SUITE_LOG) Makefile

pkgconfiglibdir=$(libdir)/pkgconfig

BUILT_SOURCES =
CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST =
include_HEADERS =
pkgconfiglib_DATA =
noinst_LIBRARIES =
check_PROGRAMS =
default_tests =
TESTS =

AM_CPPFLAGS = \
	-include $(top_builddir)/build/config.h \
	-I $(top_srcdir)/src \
	-I $(top_builddir)/src \
	$(OUR_CPPFLAGS)

AM_CFLAGS = $(OUR_CFLAGS)
AM_LDFLAGS = $(OUR_LDFLAGS)

# ------------------------------------------------------------------------------
# API headers

include_HEADERS += \
	src/c-bitmap.h \
	src/c-list.h \
	src/c-macro.h \
	src/c-ref.h \
	src/c-string.h \
	src/c-syscall.h \
	src/c-usec.h

%.pc: %.pc.in
	$(AM_V_GEN)$(SED) \
		-e 's,@includedir\@,$(includedir),g' \
		-e 's,@libdir\@,$(libdir),g' \
		-e 's,@VERSION\@,$(VERSION),g' \
		$< > $@

pkgconfiglib_DATA += \
	src/c-sundry.pc

EXTRA_DIST += \
	src/c-sundry.pc.in

CLEANFILES += \
	src/c-sundry.pc

# ------------------------------------------------------------------------------
# test-api

default_tests += \
	test-api \
	test-bitmap \
	test-list

test_api_SOURCES = \
	src/test-api.c

test_bitmap_SOURCES = \
	src/test-bitmap.c

test_list_SOURCES = \
	src/test-list.c

# ------------------------------------------------------------------------------
# test suite

AM_TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(abs_builddir)
check_PROGRAMS += $(default_tests)
TESTS += $(default_tests)

# ------------------------------------------------------------------------------¶
# check "make install" directory tree

install-tree: all
	rm -rf $(abs_builddir)/install-tree
	$(MAKE) install DESTDIR=$(abs_builddir)/install-tree
	tree $(abs_builddir)/install-tree
.PHONY: install-tree
