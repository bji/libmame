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

LIBMAMEOBJS = $(OBJ)/libmame/hashtable.o                                     \
              $(OBJ)/libmame/libmame_idv.o                                   \
              $(OBJ)/libmame/libmame_games.o                                 \
              $(OBJ)/libmame/libmame_options.o                               \
              $(OBJ)/libmame/libmame_rungame.o


#-------------------------------------------------
# rules
#------------------------------------------------

ifdef STATIC

# Because of command line length limits in Microsoft Windows, use GNU tools
# @file option.  Unfortunately creating this file from within a rule is
# difficult because just echoing the lines to the file results in command
# lines that are too long.  I can think of no way to do this except via
# makefile hackery.

LIBMAME = $(OBJ)/libmame.a
LIBMAME_STATIC_OBJS := $(LIBMAMEOBJS) $(VERSIONOBJ) $(EMUINFOOBJ)            \
                       $(DRVLIBOBJS) $(OSDCOREOBJS) $(LIBEMUOBJS) $(CPUOBJS) \
                       $(DASMOBJS) $(SOUNDOBJS) $(LIBJPEGOBJS)               \
                       $(LIBFLACOBJS) $(LIB7ZOBJS) $(FORMATSOBJS)            \
                       $(UTILOBJS) $(EXPATOBJS) $(ZLIBOBJS)                  \
                       $(SOFTFLOATOBJS) $(DRIVLISTOBJ) $(DEVLISTOBJ) 

$(OBJ)/libmame/libmame_arargs:
		$(shell rm -f $(OBJ)/libmame/libmame_arargs)
		$(foreach word,crs $(LIBMAME) $(LIBMAME_STATIC_OBJS),$(shell echo $(word) >> $(OBJ)/libmame/libmame_arargs))

# It would be nice to somehow resolve all symbols in libmame.a, leaving only
# the LibMame_* symbols as undefined, but this doesn't seem possible with ld.
# I don't understand why ld doesn't have an option for linking a bunch of
# object files into another object file with all relocations done, and yet
# leaving unresolved symbols unresolved; but it doesn't.

$(LIBMAME): $(LIBMAME_STATIC_OBJS) | $(OBJ)/libmame/libmame_arargs
			$(ECHO) Archiving $@...
			$(AR) @$(OBJ)/libmame/libmame_arargs

else

VERSION_SCRIPT := $(SRC)/libmame/libmame.version

LIBMAME = $(OBJ)/libmame$(SHLIB)

ifdef SYMBOLS

$(LIBMAME): $(LIBMAMEOBJS) $(VERSIONOBJ) $(EMUINFOOBJ) $(DRIVLISTOBJ)        \
            $(DEVLISTOBJ) $(DRVLIBS) $(LIBOSD) $(LIBCPU) $(LIBEMU)           \
            $(LIBDASM) $(LIBSOUND) $(LIBUTIL) $(EXPAT) $(SOFTFLOAT)          \
            $(JPEG_LIB) $(FLAC_LIB) $(7Z_LIB) $(FORMATS_LIB) $(LIBOCORE)     \
            $(ZLIB) $(RESFILE)
			$(ECHO) Linking $@...
			$(LD) $(LDFLAGS) -shared -o $@ $^ -lpthread

else

$(LIBMAME): $(LIBMAMEOBJS) $(VERSIONOBJ) $(EMUINFOOBJ) $(DRIVLISTOBJ)        \
            $(DEVLISTOBJ) $(DRVLIBS) $(LIBOSD) $(LIBCPU) $(LIBEMU)           \
            $(LIBDASM) $(LIBSOUND) $(LIBUTIL) $(EXPAT) $(SOFTFLOAT)          \
            $(JPEG_LIB) $(FLAC_LIB) $(7Z_LIB) $(FORMATS_LIB) $(LIBOCORE)     \
            $(ZLIB) $(RESFILE)
			$(ECHO) Linking $@...
			$(LD) $(LDFLAGS) -shared -Wl,--version-script=$(VERSION_SCRIPT)  \
                -o $@ $^ -lpthread
			$(ECHO) Stripping $@...
			$(STRIP) --strip-unneeded $@

endif

endif


# Convenience target for libmame
.PHONY: libmame
libmame: maketree $(LIBMAME)
