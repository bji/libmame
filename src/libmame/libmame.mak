#-------------------------------------------------
# Add object dir so that this code can be built
#-------------------------------------------------

OBJDIRS += $(OBJ)/libmame


#-------------------------------------------------
# rule for building libmame
#-------------------------------------------------

# Consider removing expatobjs and zlibobjs from here, and instead require
# applications to link against those libraries externally.
LIBMAMECONTENTS = $(DRVOBJS) $(LIBEMUOBJS) $(CPUOBJS) $(DASMOBJS) \
                  $(SOUNDOBJS) $(UTILOBJS) $(EXPATOBJS) $(ZLIBOBJS) \
                  $(OSDCOREOBJS) $(VERSIONOBJ) \
                  $(OBJ)/libmame/HashTable.o \
                  $(OBJ)/libmame/libmame_dv.o \
                  $(OBJ)/libmame/libmame_games.o

LIBMAME_INSTALL = libmame

$(LIBMAME_INSTALL)/usr/lib/libmame.a: $(LIBMAMECONTENTS)
		   $(MD) -p `dirname $@`
		   $(ECHO) Linking $@...
		   ar crv $@ $(LIBMAMECONTENTS)

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
