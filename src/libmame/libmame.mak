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

# These are the object files and libraries whose contents will be combined
# together to make the libmame library
LIBMAMEOBJS = $(VERSIONOBJ) $(DRVLIBOBJS) $(OSDCOREOBJS) $(LIBEMUOBJS) \
              $(CPUOBJS) $(DASMOBJS) $(SOUNDOBJS) $(FORMATSOBJS) \
              $(UTILOBJS) $(EXPATOBJS) $(COTHREADOBJS) $(ZLIBOBJS) \
              $(SOFTFLOATOBJS) $(DRIVLISTOBJ) \
              $(OBJ)/libmame/hashtable.o \
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
$(LIBMAME): $(LIBMAMEOBJS)
			$(ECHO) Archiving $@...
			@echo "crs $@ $^" > $(OBJ)/libmame/arargs
			$(AR) @$(OBJ)/libmame/arargs

else

VERSION_SCRIPT := $(SRC)/libmame/libmame.version

ifdef SYMBOLS

LIBMAME = $(OBJ)/libmame.so
$(LIBMAME): $(LIBMAMEOBJS)
			$(ECHO) Linking $@...
			@echo "-shared -Wl,--version-script=$(VERSION_SCRIPT) -o $@ $^" \
                > $(OBJ)/libmame/ldargs
			$(LD) @$(OBJ)/libmame/ldargs

else

LIBMAME = $(OBJ)/libmame.so
$(LIBMAME): $(LIBMAMEOBJS)
			$(ECHO) Linking $@...
			@echo "-shared -Wl,--version-script=$(VERSION_SCRIPT) -o $@ $^" \
                > $(OBJ)/libmame/ldargs
			$(LD) @$(OBJ)/libmame/ldargs
			$(ECHO) Stripping $@...
			$(STRIP) --strip-unneeded $@

endif

endif


# Convenience target for libmame
.PHONY: libmame
libmame: maketree $(LIBMAME)
