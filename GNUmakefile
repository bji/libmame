# *****************************************************************************
# GNUmakefile
#
# Copyright 2011 Bryan Ischo
#
# *****************************************************************************

# This GNUmakefile operates in one of two modes:
#
# - Managed Mode: in this mode, the GNUmakefile is included by a managing
#   project makefile.  This GNUmakefile declares does not define an "all"
#   target.  It defines the following symbols to be used by the managing
#   makefile:
#   LIBMAME_INCLUDES: Set to the location of the libmame include files
#   LIBMAME_STATIC_LIBRARY: Set to the location of the built static
#     libmame library
#   LIBMAME_SHARED_LIBRARY: Set to the location of the built shared
#     libmame library
# - Standalone Mode: in this mode, the GNUmakefile is used independent of
#   any other project and defines the "all" target.

# Try to include GNUmakefrag from its standard location
-include /usr/share/makefrag/GNUmakefrag.v1

# Now ensure that GNUMAKEFRAG_INCLUDED is defined or else no GNUmakefrag.v1
# file was present
ifndef GNUMAKEFRAG_INCLUDED
    $(error "ERROR: Unable to find GNUmakefrag.v1")
endif

# MAME wants PTR64 to be defined on 64 bit systems
ifeq ($(TARGET_ARCH),x86_64)
    PTR64 := 1
else
ifeq ($(TARGET_ARCH),x86)
    PTR64 := 0
else
    $(error "ERROR: Unknown TARGET_ARCH: $(TARGET_ARCH)")
endif
endif

ifndef LIBMAME_PROJECT_PREFIX

LIBMAME_PROJECT_PREFIX := .

all: libmame

endif

MAME_OUTPUT_DIRECTORY := $(LIBMAME_PROJECT_PREFIX)/obj/posix/mame

ifeq ($(PTR64),1)
    MAME_OUTPUT_DIRECTORY := $(MAME_OUTPUT_DIRECTORY)64
endif

ifneq ($(DEBUG),)
    MAME_OUTPUT_DIRECTORY := $(MAME_OUTPUT_DIRECTORY)d
endif

ifneq ($(PROFILE),)
    MAME_OUTPUT_DIRECTORY := $(MAME_OUTPUT_DIRECTORY)p
endif

LIBMAME_INCLUDES := $(INSTALL_DIR)/include/libmame/libmame.h
LIBMAME_STATIC_LIBRARY := $(INSTALL_DIR)/lib/libmame.a
LIBMAME_SHARED_LIBRARY := $(INSTALL_DIR)/lib/libmame$(SE)

# For Microsoft Windows platform, include special include path and library
# link path.  Also define some stuff that pthreads-win32 needs to compile
# without warnings in 32 bit builds; weird.
LIBMAME_CFLAGS_EXTRA :=
LIBMAME_LDFLAGS_EXTRA :=
ifeq ($(BUILD_OS),mswin)
    ifeq ($(TARGET_ARCH),x86)
	    LIBMAME_CFLAGS_EXTRA := -Iz:/include -DHAVE_CONFIG_H=0 -DHAVE_SIGNAL_H=0 -DHAVE_STRUCT_TIMESPEC=1 -D_POSIX_C_SOURCE=200103
    else
	    LIBMAME_CFLAGS_EXTRA := -Iz:/include
    endif
	LIBMAME_LDFLAGS_EXTRA := -Lz:/lib
endif

# mswin libmame target puts static library in same place as shared
ifeq ($(TARGET_OS),mswin)
    LIBMAME_STATIC_LIBRARY_BUILD := $(MAME_OUTPUT_DIRECTORY)/libmame.a
else
    LIBMAME_STATIC_LIBRARY_BUILD := $(MAME_OUTPUT_DIRECTORY)s/libmame.a
endif

# -----------------------------------------------------------------------------
# Convenience targets
.PHONY: libmame
libmame: $(LIBMAME_INCLUDES)                                                  \
         $(LIBMAME_STATIC_LIBRARY)                                            \
         $(LIBMAME_SHARED_LIBRARY) 

.PHONY: libmame-static
libmame-static: $(LIBMAME_STATIC_LIBRARY)

.PHONY: libmame-shared
libmame-shared: $(LIBMAME_SHARED_LIBRARY)
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Install targets
$(INSTALL_DIR)/include/libmame/libmame.h:                                     \
                              $(LIBMAME_PROJECT_PREFIX)/src/libmame/libmame.h
$(LIBMAME_STATIC_LIBRARY): $(LIBMAME_STATIC_LIBRARY_BUILD)
$(LIBMAME_SHARED_LIBRARY): $(MAME_OUTPUT_DIRECTORY)/libmame$(SE)
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Library targets 
# Unfortunately, make recursion is necessary because the makefiles for
# MAME do not fit into the managed makefile scheme
$(LIBMAME_STATIC_LIBRARY_BUILD):
	$(VERBOSE_SHOW) $(MAKE) BUILD_LIBMAME=1 PTR64=$(PTR64) DEBUG=$(DEBUG) PROFILE=$(PROFILE) SYMBOLS=$(DEBUG) STATIC=1 CFLAGS_EXTRA="$(LIBMAME_CFLAGS_EXTRA)" LDFLAGS_EXTRA="$(LIBMAME_LDFLAGS_EXTRA)" -C $(LIBMAME_PROJECT_PREFIX) -f makefile libmame

# Unfortunately, make recursion is necessary because the makefiles for
# MAME do not fit into the managed makefile scheme
$(MAME_OUTPUT_DIRECTORY)/libmame$(SE):
	$(VERBOSE_SHOW) $(MAKE) BUILD_LIBMAME=1 PTR64=$(PTR64) DEBUG=$(DEBUG) PROFILE=$(PROFILE) SYMBOLS=$(DEBUG) STATIC= CFLAGS_EXTRA="$(LIBMAME_CFLAGS_EXTRA)" LDFLAGS_EXTRA="$(LIBMAME_LDFLAGS_EXTRA)" -C $(LIBMAME_PROJECT_PREFIX) -f makefile libmame
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Target for cleaning out libmame
.PHONY: libmame-clean
libmame-clean:
	$(MAKE) -C $(LIBMAME_PROJECT_PREFIX) -f makefile clean
# -----------------------------------------------------------------------------
