
#-------------------------------------------------
# Force PIC objects
#-------------------------------------------------

# CFLAGS += -fPIC


#-------------------------------------------------
# object and source roots
#-------------------------------------------------

OSDLIBSRC = $(SRC)/osd/lib
OSDLIBOBJ = $(OBJ)/osd/lib

OBJDIRS += $(OSDLIBOBJ)


#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
	$(OSDLIBOBJ)/HashTable.o \
    $(OSDLIBOBJ)/libmame_dv.o \
    $(OSDLIBOBJ)/libmame_games.o \
    $(OSDLIBOBJ)/osd_directory.o \
    $(OSDLIBOBJ)/osd_file.o \
    $(OSDLIBOBJ)/osd_iav.o \
    $(OSDLIBOBJ)/osd_lock.o \
    $(OSDLIBOBJ)/osd_misc.o \
    $(OSDLIBOBJ)/osd_util.o \
    $(OSDLIBOBJ)/osd_work_queue.o


#-------------------------------------------------
# OSD mini library
#-------------------------------------------------

OSDOBJS = \
    $(OSDLIBOBJ)/osd_main.o


#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

LIBS += -lpthread

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)


#-------------------------------------------------
# rule for building libmame
#-------------------------------------------------

# LIBMAMECONTENTS = $(DRVLIBS) $(LIBEMUOBJS) $(CPUOBJS) $(DASMOBJS) \
#                   $(SOUNDOBJS) $(UTILOBJS) $(EXPATOBJS) $(ZLIBOBJS) \
#                   $(OSDCOREOBJS) $(VERSIONOBJ)

# Consider removing expatobjs and zlibobjs from here, and instead require
# applications to link against those libraries externally.
LIBMAMECONTENTS = $(DRVLIBS) $(LIBEMUOBJS) $(CPUOBJS) $(SOUNDOBJS) \
                  $(UTILOBJS) $(EXPATOBJS) $(ZLIBOBJS) $(OSDCOREOBJS) \
                  $(VERSIONOBJ)

LIBMAME_INSTALL = libmame

$(LIBMAME_INSTALL)/usr/lib/libmame.a: maketree $(LIBMAMECONTENTS)
		   $(MD) -p `dirname $@`
		   ar crv $@ $(LIBMAMECONTENTS)

$(LIBMAME_INSTALL)/usr/include/libmame.h: $(OSDLIBSRC)/libmame.h
			$(MD) -p `dirname $@`
			$(CP) $< $@

.PHONY: libmame
libmame: $(LIBMAME_INSTALL)/usr/lib/libmame.a \
         $(LIBMAME_INSTALL)/usr/include/libmame.h
