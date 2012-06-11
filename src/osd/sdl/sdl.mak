###########################################################################
#
#   sdl.mak
#
#   SDL-specific makefile
#
#   Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
#   Visit http://mamedev.org for licensing and usage restrictions.
#
#   SDLMAME by Olivier Galibert and R. Belmont
#
###########################################################################

###########################################################################
#################   BEGIN USER-CONFIGURABLE OPTIONS   #####################
###########################################################################


#-------------------------------------------------
# specify build options; see each option below
# for details
#-------------------------------------------------

# uncomment and edit next line to specify a distribution
# supported debian-stable, ubuntu-intrepid

# DISTRO = debian-stable
# DISTRO = ubuntu-intrepid
# DISTRO = gcc44-generic

# uncomment next line to build without OpenGL support

# NO_OPENGL = 1

# uncomment next line to build without X11 support (TARGETOS=unix only)
# this also implies, that no debugger will be builtin.

# NO_X11 = 1

# uncomment and adapt next line to link against specific GL-Library
# this will also add a rpath to the executable
# MESA_INSTALL_ROOT = /usr/local/dfb_GL

# uncomment the next line to build a binary using
# GL-dispatching.
# This option takes precedence over MESA_INSTALL_ROOT

USE_DISPATCH_GL = 1

# The following settings are currently supported for unix only.
# There is no need to play with this option unless you are doing
# active development on sdlmame or SDL.

# uncomment the next line to compile and link against SDL2.0

# SDL_LIBVER = sdl2

# uncomment the next line to use couriersud's multi-keyboard patch for sdl2.0
# SDL2_MULTIAPI = 1

# uncomment the next line to specify where you have installed
# SDL. Equivalent to the ./configure --prefix=<path>
# SDL_INSTALL_ROOT = /usr/local/sdl13

# uncomment and change the next line to build the gtk debugger for win32
# Get what you need here: http://www.gtk.org/download-windows.html
# GTK_INSTALL_ROOT = y:/couriersud/win/gtk-32



###########################################################################
##################   END USER-CONFIGURABLE OPTIONS   ######################
###########################################################################

ifndef SDL_LIBVER
SDL_LIBVER = sdl
endif

ifdef SDL_INSTALL_ROOT
SDL_CONFIG = $(SDL_INSTALL_ROOT)/bin/$(SDL_LIBVER)-config
else
SDL_CONFIG = $(SDL_LIBVER)-config
endif

ifeq ($(SDL_LIBVER),sdl2)
DEFS += -DSDLMAME_SDL2=1
	ifeq ($(SDL2_MULTIAPI),1)
	DEFS += -DSDL2_MULTIAPI
	endif
else
DEFS += -DSDLMAME_SDL2=0
endif

ifdef NOASM
DEFS += -DSDLMAME_NOASM
endif

# bring in external flags for RPM build
CCOMFLAGS += $(OPT_FLAGS)

#-------------------------------------------------
# distribution may change things
#-------------------------------------------------

ifeq ($(DISTRO),)
DISTRO = generic
else
ifeq ($(DISTRO),debian-stable)
DEFS += -DNO_AFFINITY_NP
else
ifeq ($(DISTRO),ubuntu-intrepid)
# Force gcc-4.2 on ubuntu-intrepid
CC = @gcc -V 4.2
LD = g++-4.2
else
ifeq ($(DISTRO),gcc44-generic)
CC = @gcc -V 4.4
LD = @g++-4.4
else
$(error DISTRO $(DISTRO) unknown)
endif
endif
endif
endif

DEFS += -DDISTRO=$(DISTRO)

#-------------------------------------------------
# sanity check the configuration
#-------------------------------------------------

ifdef BIGENDIAN
X86_MIPS3_DRC =
X86_PPC_DRC =
FORCE_DRC_C_BACKEND = 1
endif

ifdef NOASM
X86_MIPS3_DRC =
X86_PPC_DRC =
FORCE_DRC_C_BACKEND = 1
endif

#-------------------------------------------------
# compile and linking flags
#-------------------------------------------------

# add SDLMAME BASE_TARGETOS definitions

ifeq ($(TARGETOS),unix)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = tc
endif

ifeq ($(TARGETOS),linux)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = tc
SDL_NETWORK = taptun
endif

ifeq ($(TARGETOS),freebsd)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = tc
DEFS += -DNO_AFFINITY_NP
LIBS += -lutil
# /usr/local/include is not considered a system include directory
# on FreeBSD. GL.h resides there and throws warnings
CCOMFLAGS += -isystem /usr/local/include
# No clue here. There is a popmessage(NULL) in uimenu.c which
# triggers a non-null format warning on FreeBSD only.
CCOMFLAGS += -Wno-format
endif

ifeq ($(TARGETOS),openbsd)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = ntc
LIBS += -lutil
endif

ifeq ($(TARGETOS),netbsd)
BASE_TARGETOS = unix
SYNC_IMPLEMENTATION = ntc
LIBS += -lutil
endif

ifeq ($(TARGETOS),solaris)
BASE_TARGETOS = unix
DEFS += -DNO_AFFINITY_NP -UHAVE_VSNPRINTF -DNO_vsnprintf
SYNC_IMPLEMENTATION = tc
endif

ifeq ($(TARGETOS),macosx)
BASE_TARGETOS = unix
DEFS += -DSDLMAME_UNIX -DSDLMAME_MACOSX -DSDLMAME_DARWIN
DEBUGOBJS = $(SDLOBJ)/debugosx.o
SYNC_IMPLEMENTATION = ntc
SDLMAIN = $(SDLOBJ)/SDLMain_tmpl.o
SDLUTILMAIN = $(SDLOBJ)/SDLMain_tmpl.o
SDL_NETWORK = pcap
MAINLDFLAGS = -Xlinker -all_load
NO_X11 = 1
ifdef BIGENDIAN
ifdef SYMBOLS
CCOMFLAGS += -mlong-branch
endif	# SYMBOLS
ifeq ($(PTR64),1)
CCOMFLAGS += -arch ppc64
LDFLAGS += -arch ppc64
else
CCOMFLAGS += -arch ppc
LDFLAGS += -arch ppc
endif
$(OBJ)/emu/cpu/tms57002/tms57002.o : CCOMFLAGS += -O0
else	# BIGENDIAN
ifeq ($(PTR64),1)
CCOMFLAGS += -arch x86_64
LDFLAGS += -arch x86_64
else
CCOMFLAGS += -m32 -arch i386
LDFLAGS += -m32 -arch i386
endif
endif	# BIGENDIAN

endif

ifeq ($(TARGETOS),win32)
BASE_TARGETOS = win32
SYNC_IMPLEMENTATION = win32
NO_X11 = 1
DEFS += -DSDLMAME_WIN32 -DX64_WINDOWS_ABI
LIBGL = -lopengl32
SDLMAIN = $(SDLOBJ)/main.o
# needed for unidasm
LDFLAGS += -Wl,--allow-multiple-definition
SDL_NETWORK = pcap

# do we have GTK ?
ifndef GTK_INSTALL_ROOT
NO_DEBUGGER = 1
else
DEBUGOBJS = $(SDLOBJ)/debugwin.o $(SDLOBJ)/dview.o $(SDLOBJ)/debug-sup.o $(SDLOBJ)/debug-intf.o
LIBS += -lgtk-win32-2.0 -lgdk-win32-2.0 -lgmodule-2.0 -lglib-2.0 -lgobject-2.0 \
	-lpango-1.0 -latk-1.0 -lgdk_pixbuf-2.0
CCOMFLAGS += -mms-bitfields
INCPATH += -I$(GTK_INSTALL_ROOT)/include/gtk-2.0 -I$(GTK_INSTALL_ROOT)/include/glib-2.0 \
	-I$(GTK_INSTALL_ROOT)/include/cairo -I$(GTK_INSTALL_ROOT)/include/pango-1.0 \
	-I$(GTK_INSTALL_ROOT)/include/atk-1.0 \
	-I$(GTK_INSTALL_ROOT)/lib/glib-2.0/include -I$(GTK_INSTALL_ROOT)/lib/gtk-2.0/include
LDFLAGS += -L$(GTK_INSTALL_ROOT)/lib
endif # GTK_INSTALL_ROOT

# enable UNICODE
DEFS += -Dmain=utf8_main -DUNICODE -D_UNICODE
LDFLAGS += -municode

endif

ifeq ($(TARGETOS),os2)
BASE_TARGETOS = os2
DEFS += -DSDLMAME_OS2
SYNC_IMPLEMENTATION = os2
NO_DEBUGGER = 1
NO_X11 = 1
# OS/2 can't have OpenGL (aww)
NO_OPENGL = 1
endif

#-------------------------------------------------
# Sanity checks
#-------------------------------------------------

ifeq ($(BASE_TARGETOS),)
$(error $(TARGETOS) not supported !)
endif

#-------------------------------------------------
# object and source roots
#-------------------------------------------------

SDLSRC = $(SRC)/osd/$(OSD)
SDLOBJ = $(OBJ)/osd/$(OSD)

OBJDIRS += $(SDLOBJ)

#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
	$(SDLOBJ)/strconv.o	\
	$(SDLOBJ)/sdldir.o	\
	$(SDLOBJ)/sdlfile.o 	\
	$(SDLOBJ)/sdlptty_$(BASE_TARGETOS).o	\
	$(SDLOBJ)/sdlsocket.o	\
	$(SDLOBJ)/sdlmisc_$(BASE_TARGETOS).o	\
	$(SDLOBJ)/sdlos_$(SDLOS_TARGETOS).o	\
	$(SDLOBJ)/sdlsync_$(SYNC_IMPLEMENTATION).o     \
	$(SDLOBJ)/sdlwork.o

# any "main" must be in LIBOSD or else the build will fail!
# for the windows build, we just add it to libocore as well.
OSDOBJS = \
	$(SDLMAIN) \
	$(SDLOBJ)/sdlmain.o \
	$(SDLOBJ)/input.o \
	$(SDLOBJ)/sound.o  \
	$(SDLOBJ)/video.o \
	$(SDLOBJ)/drawsdl.o \
	$(SDLOBJ)/window.o \
	$(SDLOBJ)/output.o \
	$(SDLOBJ)/watchdog.o

# Add SDL2.0 support

ifeq ($(SDL_LIBVER),sdl2)
OSDOBJS += $(SDLOBJ)/draw13.o
endif

# add an ARCH define
DEFS += "-DSDLMAME_ARCH=$(ARCHOPTS)" -DSYNC_IMPLEMENTATION=$(SYNC_IMPLEMENTATION)

#-------------------------------------------------
# Generic defines and additions
#-------------------------------------------------

OSDCLEAN = sdlclean

# add the debugger includes
INCPATH += -Isrc/debug

# add the prefix file
INCPATH += -include $(SDLSRC)/sdlprefix.h

#-------------------------------------------------
# BASE_TARGETOS specific configurations
#-------------------------------------------------

SDLOS_TARGETOS = $(BASE_TARGETOS)

#-------------------------------------------------
# Unix
#-------------------------------------------------
ifeq ($(BASE_TARGETOS),unix)

#-------------------------------------------------
# Mac OS X
#-------------------------------------------------

ifeq ($(TARGETOS),macosx)
OSDCOREOBJS += $(SDLOBJ)/osxutils.o
SDLOS_TARGETOS = macosx

ifndef MACOSX_USE_LIBSDL
# Compile using framework (compile using libSDL is the exception)
LIBS += -framework SDL -framework Cocoa -framework OpenGL -lpthread
else
# Compile using installed libSDL (Fink or MacPorts):
#
# Remove the "/SDL" component from the include path so that we can compile
# files (header files are #include "SDL/something.h", so the extra "/SDL"
# causes a significant problem)
INCPATH += `sdl-config --cflags | sed 's:/SDL::'`
CCOMFLAGS += -DNO_SDL_GLEXT
# Remove libSDLmain, as its symbols conflict with SDLMain_tmpl.m
LIBS += `sdl-config --libs | sed 's/-lSDLmain//'` -lpthread
DEFS += -DMACOSX_USE_LIBSDL
endif   # MACOSX_USE_LIBSDL

else   # ifeq ($(TARGETOS),macosx)

DEFS += -DSDLMAME_UNIX
DEBUGOBJS = $(SDLOBJ)/debugwin.o $(SDLOBJ)/dview.o $(SDLOBJ)/debug-sup.o $(SDLOBJ)/debug-intf.o
LIBGL = -lGL
ifeq ($(NO_X11),1)
NO_DEBUGGER = 1
endif

INCPATH += `$(SDL_CONFIG) --cflags  | sed -e 's:/SDL[2]*::' -e 's:\(-D[^ ]*\)::g'`
CCOMFLAGS += `$(SDL_CONFIG) --cflags  | sed -e 's:/SDL[2]*::' -e 's:\(-I[^ ]*\)::g'`
LIBS += -lm `$(SDL_CONFIG) --libs`

ifeq ($(SDL_LIBVER),sdl2)
ifdef SDL_INSTALL_ROOT
# FIXME: remove the directfb ref. later. This is just there for now to work around an issue with SDL1.3 and SDL2.0
INCPATH += -I$(SDL_INSTALL_ROOT)/include/directfb
endif
endif

INCPATH += `pkg-config --cflags fontconfig`
LIBS += `pkg-config --libs fontconfig`

ifeq ($(SDL_LIBVER),sdl2)
LIBS += -lSDL2_ttf -lutil
else
LIBS += -lSDL_ttf -lutil
endif

endif # not Mac OS X

ifneq (,$(findstring ppc,$(UNAME)))
# override for preprocessor weirdness on PPC Linux
CFLAGS += -Upowerpc
SUPPORTSM32M64 = 1
endif

ifneq (,$(findstring amd64,$(UNAME)))
SUPPORTSM32M64 = 1
endif
ifneq (,$(findstring x86_64,$(UNAME)))
SUPPORTSM32M64 = 1
endif
ifneq (,$(findstring i386,$(UNAME)))
SUPPORTSM32M64 = 1
endif

ifeq ($(SUPPORTSM32M64),1)
ifeq ($(PTR64),1)
CCOMFLAGS += -m64
LDFLAGS += -m64
else
CCOMFLAGS += -m32
LDFLAGS += -m32
endif
endif

endif # Unix

#-------------------------------------------------
# Windows
#-------------------------------------------------

# Win32: add the necessary libraries
ifeq ($(BASE_TARGETOS),win32)

# Add to osdcoreobjs so tools will build
OSDCOREOBJS += $(SDLMAIN)

ifdef SDL_INSTALL_ROOT
INCPATH += -I$(SDL_INSTALL_ROOT)/include
LIBS += -L$(SDL_INSTALL_ROOT)/lib
#-Wl,-rpath,$(SDL_INSTALL_ROOT)/lib
endif

# LIBS += -lmingw32 -lSDL
# Static linking

LDFLAGS += -static-libgcc
TEST_GCC = $(shell gcc --version)
ifeq ($(findstring 4.4,$(TEST_GCC)),)
	#if we use new tools
	LDFLAGS += -static-libstdc++
endif
LIBS += -lSDL.dll
LIBS += -luser32 -lgdi32 -lddraw -ldsound -ldxguid -lwinmm -ladvapi32 -lcomctl32 -lshlwapi

endif	# Win32

#-------------------------------------------------
# OS/2
#-------------------------------------------------

ifeq ($(BASE_TARGETOS),os2)

INCPATH += `sdl-config --cflags`
LIBS += `sdl-config --libs`

endif # OS2

#-------------------------------------------------
# Debugging
#-------------------------------------------------

ifeq ($(NO_DEBUGGER),1)
DEFS += -DNO_DEBUGGER
# debugwin compiles into a stub ...
OSDOBJS += $(SDLOBJ)/debugwin.o
else
OSDOBJS += $(DEBUGOBJS)
endif # NO_DEBUGGER

#-------------------------------------------------
# OPENGL
#-------------------------------------------------

ifeq ($(NO_OPENGL),1)
DEFS += -DUSE_OPENGL=0
else
OSDOBJS += $(SDLOBJ)/drawogl.o $(SDLOBJ)/gl_shader_tool.o $(SDLOBJ)/gl_shader_mgr.o
DEFS += -DUSE_OPENGL=1
ifeq ($(USE_DISPATCH_GL),1)
DEFS += -DUSE_DISPATCH_GL=1
else
LIBS += $(LIBGL)
endif
endif

ifneq ($(USE_DISPATCH_GL),1)
ifdef MESA_INSTALL_ROOT
LIBS += -L$(MESA_INSTALL_ROOT)/lib
LDFLAGS += -Wl,-rpath=$(MESA_INSTALL_ROOT)/lib
INCPATH += -I$(MESA_INSTALL_ROOT)/include
endif
endif

#-------------------------------------------------
# X11
#-------------------------------------------------

ifeq ($(NO_X11),1)
DEFS += -DSDLMAME_NO_X11
else
# Default libs
DEFS += -DSDLMAME_X11
LIBS += -lX11 -lXinerama

# the new debugger relies on GTK+ in addition to the base SDLMAME needs
# Non-X11 builds can not use the debugger
INCPATH += `pkg-config --cflags-only-I gtk+-2.0` `pkg-config --cflags-only-I gconf-2.0`
CCOMFLAGS += `pkg-config --cflags-only-other gtk+-2.0` `pkg-config --cflags-only-other gconf-2.0`
LIBS += `pkg-config --libs gtk+-2.0` `pkg-config --libs gconf-2.0`
#CCOMFLAGS += -DGTK_DISABLE_DEPRECATED

# some systems still put important things in a different prefix
LIBS += -L/usr/X11/lib -L/usr/X11R6/lib -L/usr/openwin/lib
# make sure we can find X headers
INCPATH += -I/usr/X11/include -I/usr/X11R6/include -I/usr/openwin/include
endif # NO_X11

#-------------------------------------------------
# Network (TAP/TUN)
#-------------------------------------------------

ifdef USE_NETWORK
ifeq ($(SDL_NETWORK),taptun)
OSDOBJS += \
	$(SDLOBJ)/netdev.o \
	$(SDLOBJ)/netdev_tap.o

DEFS += -DSDLMAME_NETWORK -DSDLMAME_NET_TAPTUN
endif

ifeq ($(SDL_NETWORK),pcap)
OSDOBJS += \
	$(SDLOBJ)/netdev.o \
	$(SDLOBJ)/netdev_pcap.o

DEFS += -DSDLMAME_NETWORK -DSDLMAME_NET_PCAP
ifneq ($(TARGETOS),win32)
LIBS += -lpcap
endif
endif
endif

#-------------------------------------------------
# Dependencies
#-------------------------------------------------

# due to quirks of using /bin/sh, we need to explicitly specify the current path
CURPATH = ./

ifeq ($(BASE_TARGETOS),os2)
# to avoid name clash of '_brk'
$(OBJ)/emu/cpu/h6280/6280dasm.o : CDEFS += -D__STRICT_ANSI__
endif # OS2

ifeq ($(TARGETOS),solaris)
# solaris only has gcc-4.3 by default and is reporting a false positive
$(OBJ)/emu/video/tms9927.o : CCOMFLAGS += -Wno-error
endif # solaris

# drawSDL depends on the core software renderer, so make sure it exists
$(SDLOBJ)/drawsdl.o : $(SRC)/emu/rendersw.c $(SDLSRC)/drawogl.c
$(SDLOBJ)/drawogl.o : $(SDLSRC)/texcopy.c $(SDLSRC)/texsrc.h

# draw13 depends on blit13.h
$(SDLOBJ)/draw13.o : $(SDLSRC)/blit13.h

#$(OSDCOREOBJS): $(SDLSRC)/sdl.mak

#$(OSDOBJS): $(SDLSRC)/sdl.mak

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)

#-------------------------------------------------
# Tools
#-------------------------------------------------

TOOLS += \
	testkeys$(EXE)

$(SDLOBJ)/testkeys.o: $(SDLSRC)/testkeys.c
	$(ECHO) Compiling $<...
	$(CC)  $(CFLAGS) $(DEFS) -c $< -o $@

TESTKEYSOBJS = \
	$(SDLOBJ)/testkeys.o \

testkeys$(EXE): $(TESTKEYSOBJS) $(LIBUTIL) $(LIBOCORE) $(SDLUTILMAIN)
	$(ECHO) Linking $@...
	$(LD) $(LDFLAGS) $^ $(LIBS) -o $@

#-------------------------------------------------
# clean up
#-------------------------------------------------

$(OSDCLEAN):
	@rm -f .depend_*

#-------------------------------------------------
# various support targets
#-------------------------------------------------

testlib:
	@echo LIBS: $(LIBS)
	@echo INCPATH: $(INCPATH)
	@echo DEFS: $(DEFS)
	@echo CORE: $(OSDCOREOBJS)

ifneq ($(TARGETOS),win32)
BUILD_VERSION = $(shell grep 'build_version\[\] =' src/version.c | sed -e "s/.*= \"//g" -e "s/ .*//g")
DISTFILES = test_dist.sh whatsnew.txt whatsnew_$(BUILD_VERSION).txt makefile  docs/ src/
EXCLUDES = -x "*/.svn/*"

zip:
	zip -rq ../mame_$(BUILD_VERSION).zip $(DISTFILES) $(EXCLUDES)

endif
