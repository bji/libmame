/**
 * Implementation of osd functions from osdefault.h
 **/

void osd_init(running_machine *machine);

void osd_update(running_machine *machine, int skip_redraw);

void osd_update_audio_stream(running_machine *machine, INT16 *buffer, int samples_this_frame);

void osd_set_mastervolume(int attenuation);

void osd_customize_input_type_list(input_type_desc *typelist);
