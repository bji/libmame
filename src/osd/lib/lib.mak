
#-------------------------------------------------
# Force PIC objects
#-------------------------------------------------

# CFLAGS += -fPIC


#-------------------------------------------------
# object and source roots
#-------------------------------------------------

OSDLIBOBJ = $(OBJ)/osd/$(OSD)

OBJDIRS += $(OSDLIBOBJ)


#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
    $(OSDLIBOBJ)/osd_directory.o \
    $(OSDLIBOBJ)/osd_file.o \
    $(OSDLIBOBJ)/osd_lib.o \
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

LIBMAMECONTENTS = $(DRVLIBS) $(LIBEMUOBJS) $(CPUOBJS) $(SOUNDOBJS) \
                  $(UTILOBJS) $(EXPATOBJS) $(ZLIBOBJS) $(OSDCOREOBJS) \
                  $(VERSIONOBJ)

LIBMAME_TARGET = libmame.a

$(LIBMAME_TARGET): maketree buildtools $(LIBMAMECONTENTS)
		   ar crv $@ $(LIBMAMECONTENTS)
