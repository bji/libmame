/*********************************************************************

    cassette.c

    Interface to the cassette image abstraction code

*********************************************************************/

#include "emu.h"
#include "formats/imageutl.h"
#include "cassette.h"
#include "ui.h"


#define ANIMATION_FPS		1
#define ANIMATION_FRAMES	4

#define VERBOSE				0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/* Default cassette_interface for drivers only wav files */
const cassette_interface default_cassette_interface =
{
	cassette_default_formats,
	NULL,
	CASSETTE_PLAY,
	NULL,
	NULL
};


// device type definition
const device_type CASSETTE = &device_creator<cassette_image_device>;

//-------------------------------------------------
//  cassette_image_device - constructor
//-------------------------------------------------

cassette_image_device::cassette_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, CASSETTE, "Cassette", tag, owner, clock),
	  device_image_interface(mconfig, *this)
{

}

//-------------------------------------------------
//  cassette_image_device - destructor
//-------------------------------------------------

cassette_image_device::~cassette_image_device()
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void cassette_image_device::device_config_complete()
{
	// inherit a copy of the static data
	const cassette_interface *intf = reinterpret_cast<const cassette_interface *>(static_config());
	if (intf != NULL)
		*static_cast<cassette_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_formats, 0, sizeof(m_formats));
		memset(&m_create_opts, 0, sizeof(m_create_opts));
		memset(&m_default_state, 0, sizeof(m_default_state));
		memset(&m_interface, 0, sizeof(m_interface));
		memset(&m_device_displayinfo, 0, sizeof(m_device_displayinfo));
	}

	m_extension_list[0] = '\0';
	for (int i = 0; m_formats[i]; i++ )
		image_specify_extension( m_extension_list, 256, m_formats[i]->extensions );

	// set brief and instance name
	update_names();
}


/*********************************************************************
    cassette IO
*********************************************************************/

bool cassette_image_device::is_motor_on()
{
	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_STOPPED)
		return FALSE;
	if ((m_state & CASSETTE_MASK_MOTOR) != CASSETTE_MOTOR_ENABLED)
		return FALSE;
	return TRUE;
}



void cassette_image_device::update()
{
	double cur_time = device().machine().time().as_double();

	if (is_motor_on())
	{
		double new_position = m_position + (cur_time - m_position_time);

		switch(m_state & CASSETTE_MASK_UISTATE) {
		case CASSETTE_RECORD:
			cassette_put_sample(m_cassette, 0, m_position, new_position - m_position, m_value);
			break;

		case CASSETTE_PLAY:
			if ( m_cassette )
			{
				cassette_get_sample(m_cassette, 0, new_position, 0.0, &m_value);
				/* See if reached end of tape */
				double length = get_length();
				if (new_position > length)
				{
					m_state = (cassette_state)(( m_state & ~CASSETTE_MASK_UISTATE ) | CASSETTE_STOPPED);
					new_position = length;
				}
			}
			break;
		}
		m_position = new_position;
	}
	m_position_time = cur_time;
}

void cassette_image_device::change_state(cassette_state state, cassette_state mask)
{
	cassette_state new_state;

	new_state = m_state;
	new_state = (cassette_state)(new_state & ~mask);
	new_state = (cassette_state)(new_state | (state & mask));

	if (new_state != m_state)
	{
		update();
		m_state = new_state;
	}
}



double cassette_image_device::input()
{
	INT32 sample;
	double double_value;

	update();
	sample = m_value;
	double_value = sample / ((double) 0x7FFFFFFF);

	LOG(("cassette_input(): time_index=%g value=%g\n", m_position, double_value));

	return double_value;
}



void cassette_image_device::output(double value)
{
	if (((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD) && (m_value != value))
	{
		update();

		value = MIN(value, 1.0);
		value = MAX(value, -1.0);

		m_value = (INT32) (value * 0x7FFFFFFF);
	}
}


double cassette_image_device::get_position()
{
	double position = m_position;

	if (is_motor_on())
		position += device().machine().time().as_double() - m_position_time;
	return position;
}



double cassette_image_device::get_length()
{
	struct CassetteInfo info;

	cassette_get_info(m_cassette, &info);
	return ((double) info.sample_count) / info.sample_frequency;
}



void cassette_image_device::seek(double time, int origin)
{
	double length;

	update();

	length = get_length();

	switch(origin) {
	case SEEK_SET:
		break;

	case SEEK_END:
		time += length;
		break;

	case SEEK_CUR:
		time += get_position();
		break;
	}

	/* clip position into legal bounds */
	if (time < 0)
		time = 0;
	else
	if (time > length)
		time = length;

	m_position = time;
}



/*********************************************************************
    cassette device init/load/unload/specify
*********************************************************************/

void cassette_image_device::device_start()
{
	/* set to default state */
	m_cassette = NULL;
	m_state = m_default_state;
}

bool cassette_image_device::call_create(int format_type, option_resolution *format_options)
{
	return call_load();
}

bool cassette_image_device::call_load()
{
	casserr_t err;
	int cassette_flags;
	const char *extension;
	int is_writable;
	device_image_interface *image = NULL;
	interface(image);

	if (has_been_created())
	{
		/* creating an image */
		err = cassette_create((void *)image, &image_ioprocs, &wavfile_format, m_create_opts, CASSETTE_FLAG_READWRITE|CASSETTE_FLAG_SAVEONEXIT, &m_cassette);
		if (err)
			goto error;
	}
	else
	{
		/* opening an image */
		do
		{
			is_writable = !is_readonly();
			cassette_flags = is_writable ? (CASSETTE_FLAG_READWRITE|CASSETTE_FLAG_SAVEONEXIT) : CASSETTE_FLAG_READONLY;
			extension = filetype();
			err = cassette_open_choices((void *)image, &image_ioprocs, extension, m_formats, cassette_flags, &m_cassette);

			/* this is kind of a hack */
			if (err && is_writable)
				make_readonly();
		}
		while(err && is_writable);

		if (err)
			goto error;
	}

	/* set to default state, but only change the UI state */
	change_state(m_default_state, CASSETTE_MASK_UISTATE);

	/* reset the position */
	m_position = 0.0;
	m_position_time = device().machine().time().as_double();

	return IMAGE_INIT_PASS;

error:
	return IMAGE_INIT_FAIL;
}



void cassette_image_device::call_unload()
{

	/* if we are recording, write the value to the image */
	if ((m_state & CASSETTE_MASK_UISTATE) == CASSETTE_RECORD)
		update();

	/* close out the cassette */
	cassette_close(m_cassette);
	m_cassette = NULL;

	/* set to default state, but only change the UI state */
	change_state(CASSETTE_STOPPED, CASSETTE_MASK_UISTATE);
}



/*
    display a small tape icon, with the current position in the tape image
*/
void cassette_image_device::call_display()
{
	char buf[65];
	float x, y;
	int n;
	double position, length;
	cassette_state uistate;
	device_t *dev;
	static const UINT8 shapes[8] = { 0x2d, 0x5c, 0x7c, 0x2f, 0x2d, 0x20, 0x20, 0x20 };

	/* abort if we should not be showing the image */
	if (!exists())
		return;
	if (!is_motor_on())
		return;

	/* figure out where we are in the cassette */
	position = get_position();
	length = get_length();
	uistate = (cassette_state)(get_state() & CASSETTE_MASK_UISTATE);

	/* choose a location on the screen */
	x = 0.2f;
	y = 0.5f;

	dev = device().machine().devicelist().first(CASSETTE );

	while ( dev && strcmp( dev->tag(), device().tag() ) )
	{
		y += 1;
		dev = dev->typenext();
	}

	y *= ui_get_line_height(device().machine()) + 2.0f * UI_BOX_TB_BORDER;
	/* choose which frame of the animation we are at */
	n = ((int) position / ANIMATION_FPS) % ANIMATION_FRAMES;
	/* Since you can have anything in a BDF file, we will use crude ascii characters instead */
	snprintf(buf, ARRAY_LENGTH(buf), "%c%c %c %02d:%02d (%04d) [%02d:%02d (%04d)]",
#if 0
	/* THE ANIMATION HASN'T WORKED SINCE 0.114 - LEFT HERE FOR REFERENCE */
	/* NEVER SEEN THE PLAY / RECORD ICONS */
	/* character pairs 2-3, 4-5, 6-7, 8-9 form little tape cassette images */
		n * 2 + 2,								/* cassette icon left */
		n * 2 + 3,								/* cassette icon right */
		(uistate == CASSETTE_PLAY) ? 16 : 14,	/* play or record icon */
#else
		shapes[n],					/* cassette icon left */
		shapes[n|4],					/* cassette icon right */
		(uistate == CASSETTE_PLAY) ? 0x50 : 0x52,	/* play (P) or record (R) */
#endif
		((int) position / 60),
		((int) position % 60),
		(int) position,
		((int) length / 60),
		((int) length % 60),
		(int) length);

	/* draw the cassette */
	ui_draw_text_box(&device().machine().render().ui_container(), buf, JUSTIFY_LEFT, x, y, UI_BACKGROUND_COLOR);
}
