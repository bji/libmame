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
              $(CPUOBJS) $(DASMOBJS) $(SOUNDOBJS) $(UTILOBJS) \
              $(EXPATOBJS) $(ZLIBOBJS) $(SOFTFLOATOBJS) \
              $(OBJ)/libmame/hashtable.o \
              $(OBJ)/libmame/libmame_dv.o \
              $(OBJ)/libmame/libmame_games.o \
              $(OBJ)/libmame/libmame_options.o \
              $(OBJ)/libmame/libmame_rungame.o


#-------------------------------------------------
# rules
#------------------------------------------------

ifdef BUILD_LIBMAME_STATIC

# TODO: Figure out how to successfully strip the unneeded symbols from
# libmame.o
LIBMAME = $(OBJ)/libmame.a
$(LIBMAME): $(LIBMAMEOBJS)
			$(ECHO) Archiving $@...
			$(AR) crs $@ $^

else

ifdef SYMBOLS

LIBMAME = $(OBJ)/libmame.so
$(LIBMAME): $(LIBMAMEOBJS)
			$(ECHO) Linking $@...
			$(LD) -shared -o $@ $^

else

LIBMAME = $(OBJ)/libmame.so
$(LIBMAME): $(LIBMAMEOBJS)
			$(ECHO) Linking $@...
			$(LD) -shared -o $@ $^
			$(ECHO) Stripping $@...
			$(STRIP) --strip-unneeded $@
endif

endif


# Convenience target for libmame
.PHONY: libmame
libmame: maketree $(LIBMAME)
