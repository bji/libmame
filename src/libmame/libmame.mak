#-------------------------------------------------
# This is the directory into which libmame will be
# compiled
#-------------------------------------------------

LIBMAME_INSTALL = libmame


#-------------------------------------------------
# Add object dir so that this code can be built
#-------------------------------------------------

OBJDIRS += $(OBJ)/libmame


#-------------------------------------------------
# rule for building libmame
#-------------------------------------------------

# Because MAME builds its own libraries for some objects, we have to
# extract those to include them as object files in libmame.  But we
# need to depend on the libraries themselves as it is not possible to
# depend on the object files (there is no variable that lists the object
# files, only a variable that lists the driver).  So we depend on the
# archives but extract the archives when linking libmame.  That's why
# the following is as complex as it is.

# These are all of the object files that can be referenced directly
# Consider removing expatobjs and zlibobjs from here, and instead require
# applications to link against those libraries externally.
LIBMAMEOBJS_STD = $(LIBEMUOBJS) $(CPUOBJS) $(DASMOBJS) \
                  $(SOUNDOBJS) $(UTILOBJS) $(EXPATOBJS) $(ZLIBOBJS) \
                  $(OSDCOREOBJS) $(VERSIONOBJ) \
                  $(OBJ)/libmame/HashTable.o \
                  $(OBJ)/libmame/libmame_dv.o \
                  $(OBJ)/libmame/libmame_games.o \
                  $(OBJ)/libmame/libmame_iav.o

# These are all dependences of libmame, which includes the individual game
# libraries and the regular objects
LIBMAMEDEPS = $(DRVLIBS) $(LIBMAMEOBJS_STD)

# The complex filters below do the following:
# - Include mamedriv.o with its full path
# then:
# - For each archive listed in DRVLIBS (filter-out mamedrv.o, which is
#   bogusly put into DRVLIBS by mame.mak):
#   - Gather the list of objects in the archive.  For each object:
#     - Find the object in driver, machine, audio, and video obj
#       subdirectories and include it too

$(LIBMAME_INSTALL)/usr/lib/libmame.a: $(LIBMAMEDEPS)
		   $(MD) -p `dirname $@`
		   $(ECHO) Archiving $@...
		   $(AR) crs $@ $(filter %mamedriv.o,$(DRVLIBS)) $(foreach object,$(foreach archive,$(filter-out %mamedriv.o,$(DRVLIBS)),$(shell ar t $(archive))),$(wildcard $(OBJ)/mame/drivers/$(object)) $(wildcard $(OBJ)/mame/machine/$(object)) $(wildcard $(OBJ)/mame/audio/$(object)) $(wildcard $(OBJ)/mame/video/$(object))) $(LIBMAMEOBJS_STD)

$(LIBMAME_INSTALL)/usr/include/libmame.h: $(SRC)/libmame/libmame.h
			$(MD) -p `dirname $@`
			$(ECHO) Copying $@...
			$(CP) $< $@

.PHONY: libmame
libmame: $(LIBMAME_INSTALL)/usr/lib/libmame.a \
         $(LIBMAME_INSTALL)/usr/include/libmame.h

libmame-test: $(OBJ)/libmame/libmame_test_main.o \
              $(LIBMAME_INSTALL)/usr/lib/libmame.a
	$(ECHO) Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) $^ $(LIBS) -o $@
