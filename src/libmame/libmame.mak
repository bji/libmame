#-------------------------------------------------
# makefile support
#-------------------------------------------------

# Add the directory in which libmame objects will
# be built to $(OBJDIRS) so the top-level makefile
# will create it
OBJDIRS += $(OBJ)/libmame


#-------------------------------------------------
# variable definitions
#-------------------------------------------------

# These are the libmame objects

LIBMAMEOBJS = $(OBJ)/libmame/hashtable.o \
              $(OBJ)/libmame/libmame_idv.o \
              $(OBJ)/libmame/libmame_games.o \
              $(OBJ)/libmame/libmame_options.o \
              $(OBJ)/libmame/libmame_rungame.o


#-------------------------------------------------
# rules
#------------------------------------------------

# Because of command line length limits in Microsoft Windows, use GNU tools
# @file option

ifdef STATIC

# It would be nice to somehow resolve all symbols in libmame.a, leaving only
# the LibMame_* symbols as undefined, but this doesn't seem possible with ld.
# I don't understand why ld doesn't have an option for linking a bunch of
# object files into another object file with all relocations done, and yet
# leaving unresolved symbols unresolved; but it doesn't.
LIBMAME = $(OBJ)/libmame.a
$(LIBMAME): $(LIBMAMEOBJS) $(VERSIONOBJ) $(DRVLIBOBJS) $(OSDCOREOBJS) \
            $(LIBEMUOBJS) $(CPUOBJS) $(DASMOBJS) $(SOUNDOBJS) $(FORMATSOBJS) \
            $(UTILOBJS) $(EXPATOBJS) $(COTHREADOBJS) $(ZLIBOBJS) \
            $(SOFTFLOATOBJS) $(DRIVLISTOBJ)
			$(ECHO) Archiving $@...
			@echo "crs $@ $^" > $(OBJ)/libmame/arargs
			$(AR) @$(OBJ)/libmame/arargs

else

VERSION_SCRIPT := $(SRC)/libmame/libmame.version

ifdef SYMBOLS

LIBMAME = $(OBJ)/libmame$(SHLIB)
$(LIBMAME): $(LIBMAMEOBJS) $(VERSIONOBJ) $(DRIVLISTOBJ) $(DRVLIBS) $(LIBOSD) \
            $(LIBCPU) $(LIBEMU) $(LIBDASM) $(LIBSOUND) $(LIBUTIL) $(EXPAT) \
            $(SOFTFLOAT) $(FORMATS_LIB) $(COTHREAD) $(LIBOCORE) $(ZLIB)
			$(ECHO) Linking $@...
			$(LD) $(LDFLAGS) -shared -Wl,--version-script=$(VERSION_SCRIPT) \
                -o $@ $^ -lpthread

else

LIBMAME = $(OBJ)/libmame$(SHLIB)
$(LIBMAME): $(LIBMAMEOBJS) $(VERSIONOBJ) $(DRIVLISTOBJ) $(DRVLIBS) $(LIBOSD) \
            $(LIBCPU) $(LIBEMU) $(LIBDASM) $(LIBSOUND) $(LIBUTIL) $(EXPAT) \
            $(SOFTFLOAT) $(FORMATS_LIB) $(COTHREAD) $(LIBOCORE) $(ZLIB)
			$(ECHO) Linking $@...
			$(LD) $(LDFLAGS) -shared -Wl,--version-script=$(VERSION_SCRIPT) \
                -o $@ $^ -lpthread
			$(ECHO) Stripping $@...
			$(STRIP) --strip-unneeded $@

endif

endif


# Convenience target for libmame
.PHONY: libmame
libmame: maketree $(LIBMAME)
