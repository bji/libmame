#-------------------------------------------------
# object and source roots
#-------------------------------------------------

OSDLIBSRC = $(SRC)/osd/posix
OSDLIBOBJ = $(OBJ)/osd/posix

OBJDIRS += $(OSDLIBOBJ)


#-------------------------------------------------
# OSD core library
#-------------------------------------------------

OSDCOREOBJS = \
    $(OSDLIBOBJ)/osd_directory.o \
    $(OSDLIBOBJ)/osd_file.o \
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
