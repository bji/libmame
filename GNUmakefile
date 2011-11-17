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
ifeq ($(BUILD_ARCH),x86_64)
    PTR64 := 1
else
ifeq ($(BUILD_ARCH),i686)
    PTR64 := 0
else
    $(error "ERROR: Unknown BUILD_ARCH: $(BUILD_ARCH)")
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

export LIBMAME_INCLUDES := $(INSTALL_DIR)/include/libmame/libmame.h
export LIBMAME_STATIC_LIBRARY := $(INSTALL_DIR)/lib/libmame.a
export LIBMAME_SHARED_LIBRARY := $(INSTALL_DIR)/lib/libmame$(SE)

# For Microsoft Windows platform, include special include path and library
# link path.  Also define some stuff that pthreads-win32 needs to compile
# without warnings.
CFLAGS_EXTRA :=
LDFLAGS_EXTRA :=
ifeq ($(BUILD_OS),mswin)
	CFLAGS_EXTRA := -Iz:/include -DHAVE_CONFIG_H=0 -DHAVE_SIGNAL_H=0 -DHAVE_STRUCT_TIMESPEC=1 -D_POSIX_C_SOURCE=200103
	LDFLAGS_EXTRA := -Lz:/lib
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
$(LIBMAME_STATIC_LIBRARY): $(MAME_OUTPUT_DIRECTORY)s/libmame.a
$(LIBMAME_SHARED_LIBRARY): $(MAME_OUTPUT_DIRECTORY)/libmame$(SE)
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Library targets 
# Unfortunately, make recursion is necessary because the makefiles for
# MAME do not fit into the managed makefile scheme
$(MAME_OUTPUT_DIRECTORY)s/libmame.a: 
	$(MAKE) BUILD_LIBMAME=1 PTR64=$(PTR64) DEBUG=$(DEBUG) PROFILE=$(PROFILE) SYMBOLS=$(DEBUG) STATIC=1 CFLAGS_EXTRA_LIBMAME=$(CFLAGS_EXTRA) LDFLAGS_EXTRA_LIBMAME=$(LDFLAGS_EXTRA) -C $(LIBMAME_PROJECT_PREFIX) -f makefile libmame

# Unfortunately, make recursion is necessary because the makefiles for
# MAME do not fit into the managed makefile scheme
$(MAME_OUTPUT_DIRECTORY)/libmame$(SE):
	$(MAKE) BUILD_LIBMAME=1 PTR64=$(PTR64) DEBUG=$(DEBUG) PROFILE=$(PROFILE) SYMBOLS=$(DEBUG) STATIC= CFLAGS_EXTRA_LIBMAME=$(CFLAGS_EXTRA) LDFLAGS_EXTRA_LIBMAME=$(LDFLAGS_EXTRA) -C $(LIBMAME_PROJECT_PREFIX) -f makefile libmame
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Target for cleaning out libmame
.PHONY: libmame-clean
libmame-clean:
	$(MAKE) -C $(LIBMAME_PROJECT_PREFIX) -f makefile clean
# -----------------------------------------------------------------------------
