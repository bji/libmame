This directory includes source which, when compiled with any OSD
implementation that implements all of the OSD functions except:

osd_init_function
osd_update_function
osd_update_audio_stream_function
osd_set_mastervolume_function
osd_customize_input_type_list_function

Will build the entirety of MAME as a libary adhering to the API
specified in libmame.h.

This is intended to allow MAME to be built as a library that any
application can link against to get access to the full functionality
of the MAME engine.
