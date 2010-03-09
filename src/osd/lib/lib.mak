
#-------------------------------------------------
# object and source roots
#-------------------------------------------------

OSDLIBSRC = $(SRC)/osd/$(OSD)
OSDLIBOBJ = $(OBJ)/osd/$(OSD)

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

OSDOBJS =


#-------------------------------------------------
# rules for building the libaries
#-------------------------------------------------

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OSDOBJS)
