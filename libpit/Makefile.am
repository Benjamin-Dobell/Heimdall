AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}
AM_CPPFLAGS = $(DEPS_CFLAGS)

lib_LIBRARIES = libpit-@LIBPIT_API_VERSION@.a

libpit_@LIBPIT_API_VERSION@_a_SOURCES = Source/libpit.cpp Source/libpit.h config.h

dist_noinst_SCRIPTS = autogen.sh
