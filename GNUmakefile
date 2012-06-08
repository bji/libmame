# *****************************************************************************
# GNUmakefile
#
# Copyright 2011 Bryan Ischo
#
# *****************************************************************************

# This GNUmakefile defines the following symbols that can be used in
# dependent projects:
# LIBMAME_HEADERS
# LIBMAME_SHARED_LIBRARY
# LIBMAME_STATIC_LIBRARY
# LIBMAME_STATIC_LIBRARY_DEPENDENCIES

# If GNUMAKEFRAG_INCLUDED is defined, then this project is being
# managed by a GNUmakefile in the superdirectory.  In that case, we
# can expect LIBMAME_PROJECT_PREFIX to already be defined, and don't have to
# define an 'all' target.
ifdef GNUMAKEFRAG_INCLUDED

# This symbol must be set to inform the managing makefile that this
# GNUmakefile was included and defined its targets.
GNUMAKEFILE_INCLUDED := 1

else

# Else, GNUMAKEFRAG_INCLUDED is not defined, so this project is being
# used in standalone mode, and must define LIBMAME_PROJECT_PREFIX and an 'all'
# target.

# First try to include the gnumakefrag project's GNUmakefrag.v1 if it is
# present as a managed project
-include ../gnumakefrag/GNUmakefrag.v1

# If it is not, try to include it in its installed location
ifndef GNUMAKEFRAG_INCLUDED
-include /usr/share/makefrag/GNUmakefrag.v1
endif

# Now ensure that GNUMAKEFRAG_INCLUDED is defined or else no GNUmakefrag.v1
# file was present
ifndef GNUMAKEFRAG_INCLUDED
    $(error "ERROR: Unable to find GNUmakefrag.v1")
endif

# Set PROJECT_PREFIX for a standalone project
PROJECT_PREFIX := .

# Define the 'all' target for a standalone project
.PHONY: all
all: libmame

endif


# PROJECT_PREFIX must be saved into a "local" variable for use
LIBMAME_PROJECT_PREFIX := $(PROJECT_PREFIX)


# Define exported variables --------------------------------------------------
LIBMAME_HEADERS := $(INSTALL_DIR)/include/libmame/libmame.h

LIBMAME_SHARED_LIBRARY := $(INSTALL_DIR)/lib/libmame$(SE)

LIBMAME_STATIC_LIBRARY := $(INSTALL_DIR)/lib/libmame.a

LIBMAME_STATIC_LIBRARY_DEPENDENCIES := pthread


# ----------------------------------------------------------------------------
# Set up a bunch of symbols that the MAME build targets need to have defined
# ----------------------------------------------------------------------------
# MAME wants PTR64 to be defined on 64 bit systems
ifeq ($(TARGET_ARCH),x86_64)
    LIBMAME_PTR64 := 1
else
ifeq ($(TARGET_ARCH),x86)
    LIBMAME_PTR64 := 0
else
    $(error "ERROR: Unknown TARGET_ARCH: $(TARGET_ARCH)")
endif
endif

LIBMAME_OBJ := $(abspath $(BUILD_DIR))/libmame/obj

LIBMAME_OUTPUT_DIRECTORY := $(LIBMAME_OBJ)/posix

ifeq ($(LIBMAME_PTR64),1)
    LIBMAME_OUTPUT_DIRECTORY := $(LIBMAME_OUTPUT_DIRECTORY)64
endif

ifneq ($(DEBUG),)
    LIBMAME_OUTPUT_DIRECTORY := $(LIBMAME_OUTPUT_DIRECTORY)d
endif

ifneq ($(PROFILE),)
    LIBMAME_OUTPUT_DIRECTORY := $(LIBMAME_OUTPUT_DIRECTORY)p
endif

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

LIBMAME_SHARED_LIBRARY_BUILD := $(LIBMAME_OUTPUT_DIRECTORY)/libmame$(SE)

# mswin libmame target puts static library in same place as shared
ifeq ($(TARGET_SYSTEM),mswin)
    LIBMAME_STATIC_LIBRARY_BUILD := $(LIBMAME_OUTPUT_DIRECTORY)/libmame.a
else
    LIBMAME_STATIC_LIBRARY_BUILD := $(LIBMAME_OUTPUT_DIRECTORY)s/libmame.a
endif

# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# Install target dependencies on source/build
$(INSTALL_DIR)/include/libmame/libmame.h:                                    \
                             $(LIBMAME_PROJECT_PREFIX)/src/libmame/libmame.h
$(LIBMAME_SHARED_LIBRARY): $(LIBMAME_SHARED_LIBRARY_BUILD)
$(LIBMAME_STATIC_LIBRARY): $(LIBMAME_STATIC_LIBRARY_BUILD)
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# Library targets 
# Unfortunately, make recursion is necessary because the makefiles for
# MAME do not fit into the managed makefile scheme
# NOWERROR is currently required because MAME doesn't compile cleanly with
# gcc 4.7
$(LIBMAME_SHARED_LIBRARY_BUILD):
	$(VERBOSE_SHOW) $(MAKE) NOWERROR=1 BUILD_LIBMAME=1 PTR64=$(LIBMAME_PTR64) DEBUG=$(DEBUG) PROFILE=$(PROFILE) SYMBOLS=$(DEBUG) STATIC= MAME_OBJ=$(LIBMAME_OBJ) CFLAGS_EXTRA="$(LIBMAME_CFLAGS_EXTRA)" LDFLAGS_EXTRA="$(LIBMAME_LDFLAGS_EXTRA)" -C $(LIBMAME_PROJECT_PREFIX) -f makefile libmame

# Unfortunately, make recursion is necessary because the makefiles for
# MAME do not fit into the managed makefile scheme
# NOWERROR is currently required because MAME doesn't compile cleanly with
# gcc 4.7
$(LIBMAME_STATIC_LIBRARY_BUILD):
	$(VERBOSE_SHOW) $(MAKE) NOWERROR=1 BUILD_LIBMAME=1 PTR64=$(LIBMAME_PTR64) DEBUG=$(DEBUG) PROFILE=$(PROFILE) SYMBOLS=$(DEBUG) STATIC=1 MAME_OBJ=$(LIBMAME_OBJ) CFLAGS_EXTRA="$(LIBMAME_CFLAGS_EXTRA)" LDFLAGS_EXTRA="$(LIBMAME_LDFLAGS_EXTRA)" -C $(LIBMAME_PROJECT_PREFIX) -f makefile libmame
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# libmame target
.PHONY: libmame
ifdef STATIC
libmame: $(LIBMAME_HEADERS) $(LIBMAME_STATIC_LIBRARY)
else
libmame: $(LIBMAME_HEADERS) $(LIBMAME_SHARED_LIBRARY)
endif
# -----------------------------------------------------------------------------


# -----------------------------------------------------------------------------
# Target for cleaning out libmame
.PHONY: libmame-clean
libmame-clean:
	$(QUIET_ECHO) $(LIBMAME_OUTPUT_DIRECTORY): Cleaning
	$(VERBOSE_SHOW) $(RMRF) $(LIBMAME_OUTPUT_DIRECTORY)
# -----------------------------------------------------------------------------
