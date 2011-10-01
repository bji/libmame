//============================================================
//
//  minimain.c - Main function for mini OSD
//
//============================================================
//
//  Copyright Aaron Giles
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the
//  following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the
//      following disclaimer.
//    * Redistributions in binary form must reproduce the
//      above copyright notice, this list of conditions and
//      the following disclaimer in the documentation and/or
//      other materials provided with the distribution.
//    * Neither the name 'MAME' nor the names of its
//      contributors may be used to endorse or promote
//      products derived from this software without specific
//      prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND
//  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
//  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
//  EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGE (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
//  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
//  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//============================================================

#include "emu.h"
#include "osdepend.h"
#include "render.h"
#include "clifront.h"
#include "osdmini.h"


//============================================================
//  CONSTANTS
//============================================================

// we fake a keyboard with the following keys
enum
{
	KEY_ESCAPE,
	KEY_P1_START,
	KEY_BUTTON_1,
	KEY_BUTTON_2,
	KEY_BUTTON_3,
	KEY_JOYSTICK_U,
	KEY_JOYSTICK_D,
	KEY_JOYSTICK_L,
	KEY_JOYSTICK_R,
	KEY_TOTAL
};


//============================================================
//  GLOBALS
//============================================================

// a single rendering target
static render_target *our_target;

// a single input device
static input_device *keyboard_device;

// the state of each key
static UINT8 keyboard_state[KEY_TOTAL];


//============================================================
//  FUNCTION PROTOTYPES
//============================================================

static INT32 keyboard_get_state(void *device_internal, void *item_internal);


//============================================================
//  main
//============================================================

int main(int argc, char *argv[])
{
	// cli_frontend does the heavy lifting; if we have osd-specific options, we
	// create a derivative of cli_options and add our own
	cli_options options;
	mini_osd_interface osd;
	cli_frontend frontend(options, osd);
	return frontend.execute(argc, argv);
}


//============================================================
//  constructor
//============================================================

mini_osd_interface::mini_osd_interface()
{
}


//============================================================
//  destructor
//============================================================

mini_osd_interface::~mini_osd_interface()
{
}


//============================================================
//  init
//============================================================

void mini_osd_interface::init(running_machine &machine)
{
	// call our parent
	osd_interface::init(machine);

	// initialize the video system by allocating a rendering target
	our_target = machine.render().target_alloc();

	// nothing yet to do to initialize sound, since we don't have any
	// sound updates are handled by update_audio_stream() below

	// initialize the input system by adding devices
	// let's pretend like we have a keyboard device
	keyboard_device = machine.input().device_class(DEVICE_CLASS_KEYBOARD).add_device("Keyboard");
	if (keyboard_device == NULL)
		fatalerror("Error creating keyboard device");

	// our faux keyboard only has a couple of keys (corresponding to the
	// common defaults)
	keyboard_device->add_item("Esc", ITEM_ID_ESC, keyboard_get_state, &keyboard_state[KEY_ESCAPE]);
	keyboard_device->add_item("P1", ITEM_ID_1, keyboard_get_state, &keyboard_state[KEY_P1_START]);
	keyboard_device->add_item("B1", ITEM_ID_LCONTROL, keyboard_get_state, &keyboard_state[KEY_BUTTON_1]);
	keyboard_device->add_item("B2", ITEM_ID_LALT, keyboard_get_state, &keyboard_state[KEY_BUTTON_2]);
	keyboard_device->add_item("B3", ITEM_ID_SPACE, keyboard_get_state, &keyboard_state[KEY_BUTTON_3]);
	keyboard_device->add_item("JoyU", ITEM_ID_UP, keyboard_get_state, &keyboard_state[KEY_JOYSTICK_U]);
	keyboard_device->add_item("JoyD", ITEM_ID_DOWN, keyboard_get_state, &keyboard_state[KEY_JOYSTICK_D]);
	keyboard_device->add_item("JoyL", ITEM_ID_LEFT, keyboard_get_state, &keyboard_state[KEY_JOYSTICK_L]);
	keyboard_device->add_item("JoyR", ITEM_ID_RIGHT, keyboard_get_state, &keyboard_state[KEY_JOYSTICK_R]);

	// hook up the debugger log
//  add_logerror_callback(machine, output_oslog);
}


//============================================================
//  osd_update
//============================================================

void mini_osd_interface::update(bool skip_redraw)
{
	// get the minimum width/height for the current layout
	int minwidth, minheight;
	our_target->compute_minimum_size(minwidth, minheight);

	// make that the size of our target
	our_target->set_bounds(minwidth, minheight);

	// get the list of primitives for the target at the current size
	render_primitive_list &primlist = our_target->get_primitives();

	// lock them, and then render them
	primlist.acquire_lock();

	// do the drawing here
	primlist.release_lock();

	// after 5 seconds, exit
	if (machine().time() > attotime::from_seconds(5))
		machine().schedule_exit();
}


//============================================================
//  update_audio_stream
//============================================================

void mini_osd_interface::update_audio_stream(const INT16 *buffer, int samples_this_frame)
{
	// if we had actual sound output, we would copy the
	// interleaved stereo samples to our sound stream
}


//============================================================
//  set_mastervolume
//============================================================

void mini_osd_interface::set_mastervolume(int attenuation)
{
	// if we had actual sound output, we would adjust the global
	// volume in response to this function
}


//============================================================
//  customize_input_type_list
//============================================================

void mini_osd_interface::customize_input_type_list(simple_list<input_type_entry> &typelist)
{
	// This function is called on startup, before reading the
	// configuration from disk. Scan the list, and change the
	// default control mappings you want. It is quite possible
	// you won't need to change a thing.
}


//============================================================
//  keyboard_get_state
//============================================================

static INT32 keyboard_get_state(void *device_internal, void *item_internal)
{
	// this function is called by the input system to get the current key
	// state; it is specified as a callback when adding items to input
	// devices
	UINT8 *keystate = (UINT8 *)item_internal;
	return *keystate;
}
