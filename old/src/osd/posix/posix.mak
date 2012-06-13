#-------------------------------------------------
# Stuff to satisfy MAME top-level makefile: making
# this OSD implementation work just like other OSD
# implementations, at least from a build perspective
#-------------------------------------------------

# Anything linking the posix OSD depends on pthreads; set this variable so
# that the top-level makefile's rules for linking executables will get this
# flag.
LIBS += -lpthread


# This causes MAME's 'maketree' target to make the posix object directory
OBJDIRS += $(OBJ)/osd/posix


# LIBOCORE is defined by the top-level makefile as the target for the 
# "OSD core" library.  Our responsibility is to set the dependencies up as
# the list of object files that this library will contain.
OSDCOREOBJS = $(OBJ)/osd/posix/osd_directory.o \
              $(OBJ)/osd/posix/osd_file.o \
              $(OBJ)/osd/posix/osd_lock.o \
              $(OBJ)/osd/posix/osd_misc.o \
              $(OBJ)/osd/posix/osd_util.o \
              $(OBJ)/osd/posix/osd_work_queue.o
$(LIBOCORE): $(OSDCOREOBJS)


# LIBOSD is defined by the top-level makefile as the target for the "OSD main"
# library, which includes the main function.  Our responsibiltiy is to set the
# dependencies up as the list of object files that this library will contain.
# We provide a dummy main that just emits an error message when run (this is
# so that the standard MAME "make emulator" command works, even if what it
# produces is not that useful).
OSDOBJS = $(OBJ)/osd/posix/osd_main.o
$(LIBOSD): $(OSDOBJS)


# test program
work_queue_test: $(MAKETREE) $(OBJ)/osd/posix/work_queue_test.o $(LIBOCORE)
				 $(ECHO) Linking $@...
				 $(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) $^ $(LIBS) -o $@
