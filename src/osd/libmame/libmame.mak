#-------------------------------------------------
# stuff that has to be done first
#-------------------------------------------------

# Anything linking libmame depends on pthreads
LIBS += -lpthread

# This causes MAME's 'maketree' target to make the libmame
# object directory
OBJDIRS += $(OBJ)/osd/libmame

# Only include dependency on 'maketree', which creates the
# object directories, if NO_MAKETREE is not set.  'maketree'
# is problematic because it causes everything to be rebuilt
# constantly and I find it better to just do make maketree
# once and then use NO_MAKETREE after that to ensure that
# it never messes things up again
ifndef NO_MAKETREE
MAKETREE = maketree
endif


#-------------------------------------------------
# rules for building libmame
#-------------------------------------------------

# Because MAME builds its own libraries for some objects, we have to
# extract those to include them as object files in libmame.  But we
# need to depend on the libraries themselves as it is not possible to
# depend on the object files (there is no variable that lists the object
# files, only a variable that lists the driver).  So we depend on the
# archives but extract the archives when linking libmame.  That's why
# the following is as complex as it is.

OSDCOREOBJS = $(OBJ)/osd/libmame/osd_directory.o \
              $(OBJ)/osd/libmame/osd_file.o \
              $(OBJ)/osd/libmame/osd_lock.o \
              $(OBJ)/osd/libmame/osd_misc.o \
              $(OBJ)/osd/libmame/osd_stubs.o \
              $(OBJ)/osd/libmame/osd_util.o \
              $(OBJ)/osd/libmame/osd_work_queue.o

# These are all of the object files that can be referenced directly
# Consider removing expatobjs and zlibobjs from here, and instead require
# applications to link against those libraries externally.
LIBMAMEOBJS_STD = $(LIBEMUOBJS) $(CPUOBJS) $(DASMOBJS) \
                  $(SOUNDOBJS) $(UTILOBJS) $(EXPATOBJS) $(ZLIBOBJS) \
                  $(SOFTFLOATOBJS) $(OSDCOREOBJS) $(VERSIONOBJ) \
                  $(OBJ)/osd/libmame/HashTable.o \
                  $(OBJ)/osd/libmame/libmame_dv.o \
                  $(OBJ)/osd/libmame/libmame_games.o \
                  $(OBJ)/osd/libmame/libmame_options.o \
                  $(OBJ)/osd/libmame/libmame_rungame.o

# These are all dependences of libmame, which includes the individual game
# libraries and the regular objects, also LIBDASM because otherwise that
# library does not get built automatically
LIBMAMEDEPS = $(DRVLIBS) $(LIBMAMEOBJS_STD) $(LIBDASM)

# The complex filters below do the following:
# - Include mamedriv.o with its full path
# then:
# - For each file listed in DRVLIBS which is an object file (filter %.o),
#   include it in libmame directly
# - For each file listed in DRVLIBS which is a library file (filter %.a),
#   (bogusly put into DRVLIBS by mame.mak):
#   - Gather the list of objects in the archive.  For each object:
#     - Find the object in driver, machine, audio, and video obj
#       subdirectories and include it too

LIBMAME = libmame$(SUFFIX)$(SUFFIX64)$(SUFFIXDEBUG)$(SUFFIXGPROF).a

# Unfortunately, the MAME makefiles are set up such that it's not really
# possible for OSD makefiles to reference variables from other makefiles.
# This is unfortunate as it means that I have to depend directly on
# softfloat.o here as for some reason it's the only .o that doesn't get
# built before the libmame target for some reason.  I actually have no
# idea how the rest of this works properly given that all of the
# variables that this makefile is depending on are not even defined at
# the time that libmame.mak is included by the top level makefile.  It seems
# to me that MAME really ought to split its makefiles into definition
# makefiles (included first), and rules makefiles (included last).  That would
# provide a means for this all to work, I think.
$(LIBMAME): $(MAKETREE) $(LIBMAMEDEPS) $(OBJ)/lib/softfloat/softfloat.o
		    $(ECHO) Archiving $@...
		    $(AR) crs $@ $(filter %.o,$(DRVLIBS)) $(foreach object,$(foreach archive,$(filter %.a,$(DRVLIBS)),$(shell ar t $(archive))),$(wildcard $(OBJ)/mame/drivers/$(object)) $(wildcard $(OBJ)/mame/machine/$(object)) $(wildcard $(OBJ)/mame/audio/$(object)) $(wildcard $(OBJ)/mame/video/$(object))) $(LIBMAMEOBJS_STD)


LIBMAME_TEST = libmame-test$(SUFFIX)$(SUFFIX64)$(SUFFIXDEBUG)

$(LIBMAME_TEST): $(MAKETREE) $(OBJ)/osd/libmame/libmame_test_main.o $(LIBMAME)
	$(ECHO) Linking $@...
	$(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) $^ $(LIBS) -o $@

work_queue_test: $(MAKETREE) $(OBJ)/osd/libmame/work_queue_test.o $(LIBMAME)
				 $(ECHO) Linking $@...
				 $(LD) $(LDFLAGS) $(LDFLAGSEMULATOR) $^ $(LIBS) -o $@


#-------------------------------------------------
# phony convenience targets
#-------------------------------------------------

.PHONY: libmame
libmame: $(LIBMAME)

.PHONY: libmame-test
libmame-test: $(LIBMAME_TEST)


#-------------------------------------------------
# rules for building the standard MAME libaries
#-------------------------------------------------

$(LIBOCORE): $(OSDCOREOBJS)

$(LIBOSD): $(OBJ)/osd/libmame/osd_main.o
