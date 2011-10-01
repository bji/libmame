/*************************************************************************

    ldcore.c

    Private core laserdisc player implementation.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*************************************************************************/

#include "emu.h"
#include "ldcore.h"
#include "avcomp.h"
#include "vbiparse.h"
#include "config.h"
#include "render.h"



/***************************************************************************
    DEBUGGING
***************************************************************************/

#define LOG_SLIDER					0



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* these specs code from IEC 60857, for NTSC players */
#define LEAD_IN_MIN_RADIUS_IN_UM	53500		/* 53.5 mm */
#define PROGRAM_MIN_RADIUS_IN_UM	55000		/* 55 mm */
#define PROGRAM_MAX_RADIUS_IN_UM	145000		/* 145 mm */
#define LEAD_OUT_MIN_SIZE_IN_UM		2000		/* 2 mm */

/* the track pitch is defined as a range; we pick a nominal pitch
   that ensures we can fit 54,000 tracks */
#define MIN_TRACK_PITCH_IN_NM		1400		/* 1.4 um */
#define MAX_TRACK_PITCH_IN_NM		2000		/* 2 um */
#define NOMINAL_TRACK_PITCH_IN_NM	((PROGRAM_MAX_RADIUS_IN_UM - PROGRAM_MIN_RADIUS_IN_UM) * 1000 / 54000)

/* we simulate extra lead-in and lead-out tracks */
#define VIRTUAL_LEAD_IN_TRACKS		((PROGRAM_MIN_RADIUS_IN_UM - LEAD_IN_MIN_RADIUS_IN_UM) * 1000 / NOMINAL_TRACK_PITCH_IN_NM)
#define MAX_TOTAL_TRACKS			54000
#define VIRTUAL_LEAD_OUT_TRACKS		(LEAD_OUT_MIN_SIZE_IN_UM * 1000 / NOMINAL_TRACK_PITCH_IN_NM)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/* video frame data */
typedef struct _frame_data frame_data;
struct _frame_data
{
	bitmap_t *			bitmap;					/* cached bitmap */
	bitmap_t *			visbitmap;				/* wrapper around bitmap with only visible lines */
	UINT8				numfields;				/* number of fields in this frame */
	INT32				lastfield;				/* last absolute field number */
};


/* core-specific data */
struct _ldcore_data
{
	/* general config */
	laserdisc_config	config;					/* copy of the inline config */
	ldplayer_interface	intf;					/* interface to the player */

	/* disc parameters */
	chd_file *			disc;					/* handle to the disc itself */
	UINT8 *				vbidata;				/* pointer to precomputed VBI data */
	int					width;					/* width of video */
	int					height;					/* height of video */
	UINT32				fps_times_1million;		/* frame rate of video */
	int					samplerate;				/* audio samplerate */
	chd_error			readresult;				/* result of the most recent read */
	UINT32				chdtracks;				/* number of tracks in the CHD */
	av_codec_decompress_config avconfig;		/* decompression configuration */

	/* core states */
	UINT8				audiosquelch;			/* audio squelch state: bit 0 = audio 1, bit 1 = audio 2 */
	UINT8				videosquelch;			/* video squelch state: bit 0 = on/off */
	UINT8				fieldnum;				/* field number (0 or 1) */
	INT32				curtrack;				/* current track at this end of this vsync */
	UINT32				maxtrack;				/* maximum track number */
	attoseconds_t		attospertrack;			/* attoseconds per track, or 0 if not moving */
	attotime			sliderupdate;			/* time of last slider update */

	/* video data */
	frame_data			frame[3];				/* circular list of frames */
	UINT8				videoindex;				/* index of the current video buffer */
	bitmap_t			videotarget;			/* fake target bitmap for decompression */
	bitmap_t *			emptyframe;				/* blank frame */

	/* audio data */
	INT16 *				audiobuffer[2];			/* buffer for audio samples */
	UINT32				audiobufsize;			/* size of buffer */
	UINT32				audiobufin;				/* input index */
	UINT32				audiobufout;			/* output index */
	UINT32				audiocursamples;		/* current samples this track */
	UINT32				audiomaxsamples;		/* maximum samples per track */
	device_t *audiocustom;			/* custom sound device */

	/* metadata */
	vbi_metadata		metadata[2];			/* metadata parsed from the stream, for each field */

	/* I/O data */
	UINT8				datain;					/* current input data value */
	UINT8				linein[LASERDISC_INPUT_LINES]; /* current input line state */
	UINT8				dataout;				/* current output data value */
	UINT8				lineout[LASERDISC_OUTPUT_LINES]; /* current output line state */

	/* video updating */
	UINT8				videoenable;			/* is video enabled? */
	render_texture *	videotex;				/* texture for the video */
	palette_t *			videopalette;			/* palette for the video */
	UINT8				overenable;				/* is the overlay enabled? */
	bitmap_t *			overbitmap[2];			/* overlay bitmaps */
	int					overindex;				/* index of the overlay bitmap */
	render_texture *	overtex;				/* texture for the overlay */
};


/* sound callback info */
typedef struct _sound_token sound_token;
struct _sound_token
{
	sound_stream *		stream;
	laserdisc_state *	ld;
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* generic helper functions */
static TIMER_CALLBACK( perform_player_update );
static void read_track_data(laserdisc_state *ld);
static void process_track_data(device_t *device);
static DEVICE_START( laserdisc_sound );
static STREAM_UPDATE( custom_stream_callback );
static void configuration_load(device_t *device, int config_type, xml_data_node *parentnode);
static void configuration_save(device_t *device, int config_type, xml_data_node *parentnode);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, a laserdisc device
-------------------------------------------------*/

INLINE laserdisc_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device_is_laserdisc(device));

	return (laserdisc_state *)downcast<legacy_device_base *>(device)->token();
}


/*-------------------------------------------------
    update_audio - update the audio stream to the
    current time
-------------------------------------------------*/

INLINE void update_audio(laserdisc_state *ld)
{
	ldcore_data *ldcore = ld->core;
	if (ldcore->audiocustom != NULL)
	{
		sound_token *token = (sound_token *)downcast<legacy_device_base *>(ldcore->audiocustom)->token();
		token->stream->update();
	}
}


/*-------------------------------------------------
    add_and_clamp_track - add a delta to the
    current track and clamp to minimum/maximum
    values
-------------------------------------------------*/

INLINE void add_and_clamp_track(ldcore_data *ldcore, INT32 delta)
{
	ldcore->curtrack += delta;
	ldcore->curtrack = MAX(ldcore->curtrack, 1);
	ldcore->curtrack = MIN(ldcore->curtrack, ldcore->maxtrack - 1);
}


/*-------------------------------------------------
    fillbitmap_yuy16 - fill a YUY16 bitmap with a
    given color pattern
-------------------------------------------------*/

INLINE void fillbitmap_yuy16(bitmap_t *bitmap, UINT8 yval, UINT8 cr, UINT8 cb)
{
	UINT16 color0 = (yval << 8) | cb;
	UINT16 color1 = (yval << 8) | cr;
	int x, y;

	/* write 32 bits of color (2 pixels at a time) */
	for (y = 0; y < bitmap->height; y++)
	{
		UINT16 *dest = (UINT16 *)bitmap->base + y * bitmap->rowpixels;
		for (x = 0; x < bitmap->width / 2; x++)
		{
			*dest++ = color0;
			*dest++ = color1;
		}
	}
}



/***************************************************************************
    GENERIC IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    update_slider_pos - based on the current
    speed and elapsed time, update the current
    track position
-------------------------------------------------*/

static void update_slider_pos(ldcore_data *ldcore, attotime curtime)
{
	/* if not moving, update to now */
	if (ldcore->attospertrack == 0)
		ldcore->sliderupdate = curtime;

	/* otherwise, compute the number of tracks covered */
	else
	{
		attoseconds_t delta = (curtime - ldcore->sliderupdate).as_attoseconds();
		INT32 tracks_covered;

		/* determine how many tracks we covered and advance */
		if (ldcore->attospertrack >= 0)
		{
			tracks_covered = delta / ldcore->attospertrack;
			add_and_clamp_track(ldcore, tracks_covered);
			if (tracks_covered != 0)
				ldcore->sliderupdate += attotime(0, tracks_covered * ldcore->attospertrack);
		}
		else
		{
			tracks_covered = delta / -ldcore->attospertrack;
			add_and_clamp_track(ldcore, -tracks_covered);
			if (tracks_covered != 0)
				ldcore->sliderupdate += attotime(0, tracks_covered * -ldcore->attospertrack);
		}
	}
}


/*-------------------------------------------------
    vblank_state_changed - called on each state
    change of the VBLANK signal
-------------------------------------------------*/

static void vblank_state_changed(device_t *device, screen_device &screen, bool vblank_state)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	attotime curtime = screen.machine().time();

	/* update current track based on slider speed */
	update_slider_pos(ldcore, curtime);

	/* on rising edge, process previously-read frame and inform the player */
	if (vblank_state)
	{
		/* call the player's VSYNC callback */
		if (ldcore->intf.vsync != NULL)
			(*ldcore->intf.vsync)(ld, &ldcore->metadata[ldcore->fieldnum], ldcore->fieldnum, curtime);

		/* set a timer to begin fetching the next frame just before the VBI data would be fetched */
		screen.machine().scheduler().timer_set(screen.time_until_pos(16*2), FUNC(perform_player_update), 0, ld);
	}
}


/*-------------------------------------------------
    vblank_state_changed - called on each state
    change of the VBLANK signal
-------------------------------------------------*/

static TIMER_CALLBACK( perform_player_update )
{
	laserdisc_state *ld = (laserdisc_state *)ptr;
	ldcore_data *ldcore = ld->core;
	attotime curtime = machine.time();

	/* wait for previous read and decode to finish */
	process_track_data(ld->device);

	/* update current track based on slider speed */
	update_slider_pos(ldcore, curtime);

	/* update the state */
	if (ldcore->intf.update != NULL)
		add_and_clamp_track(ldcore, (*ldcore->intf.update)(ld, &ldcore->metadata[ldcore->fieldnum], ldcore->fieldnum, curtime));

	/* flush any audio before we read more */
	update_audio(ld);

	/* start reading the track data for the next round */
	ldcore->fieldnum ^= 1;
	read_track_data(ld);
}


/*-------------------------------------------------
    laserdisc_data_w - write data to the given
    laserdisc player
-------------------------------------------------*/

void laserdisc_data_w(device_t *device, UINT8 data)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	UINT8 prev = ldcore->datain;
	ldcore->datain = data;

	/* call through to the player-specific write handler */
	if (ldcore->intf.writedata != NULL)
		(*ldcore->intf.writedata)(ld, prev, data);
}


/*-------------------------------------------------
    laserdisc_line_w - control an input line
-------------------------------------------------*/

void laserdisc_line_w(device_t *device, UINT8 line, UINT8 newstate)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;

	assert(line < LASERDISC_INPUT_LINES);
	assert(newstate == ASSERT_LINE || newstate == CLEAR_LINE || newstate == PULSE_LINE);

	/* assert */
	if (newstate == ASSERT_LINE || newstate == PULSE_LINE)
	{
		if (ldcore->linein[line] != ASSERT_LINE)
		{
			/* call through to the player-specific line handler */
			if (ldcore->intf.writeline[line] != NULL)
				(*ldcore->intf.writeline[line])(ld, CLEAR_LINE, ASSERT_LINE);
		}
		ldcore->linein[line] = ASSERT_LINE;
	}

	/* deassert */
	if (newstate == CLEAR_LINE || newstate == PULSE_LINE)
	{
		if (ldcore->linein[line] != CLEAR_LINE)
		{
			/* call through to the player-specific line handler */
			if (ldcore->intf.writeline[line] != NULL)
				(*ldcore->intf.writeline[line])(ld, ASSERT_LINE, CLEAR_LINE);
		}
		ldcore->linein[line] = CLEAR_LINE;
	}
}


/*-------------------------------------------------
    laserdisc_data_r - return the current
    data byte
-------------------------------------------------*/

UINT8 laserdisc_data_r(device_t *device)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	UINT8 result = ldcore->dataout;

	/* call through to the player-specific data handler */
	if (ldcore->intf.readdata != NULL)
		result = (*ldcore->intf.readdata)(ld);

	return result;
}


/*-------------------------------------------------
    laserdisc_line_r - return the current state
    of an output line
-------------------------------------------------*/

UINT8 laserdisc_line_r(device_t *device, UINT8 line)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	UINT8 result;

	assert(line < LASERDISC_OUTPUT_LINES);
	result = ldcore->lineout[line];

	/* call through to the player-specific data handler */
	if (ldcore->intf.readline[line] != NULL)
		result = (*ldcore->intf.readline[line])(ld);

	return result;
}


/*-------------------------------------------------
    laserdisc_get_video - return the current
    video frame; return TRUE if valid or FALSE
    if video off
-------------------------------------------------*/

int laserdisc_get_video(device_t *device, bitmap_t **bitmap)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	frame_data *frame;

	/* determine the most recent live set of frames */
	frame = &ldcore->frame[ldcore->videoindex];
	if (frame->numfields < 2)
		frame = &ldcore->frame[(ldcore->videoindex + ARRAY_LENGTH(ldcore->frame) - 1) % ARRAY_LENGTH(ldcore->frame)];

	/* if no video present, return the empty frame */
	if (ldcore->videosquelch || frame->numfields < 2)
	{
		*bitmap = ldcore->emptyframe;
		return FALSE;
	}
	else
	{
		*bitmap = frame->visbitmap;
		return TRUE;
	}
}


/*-------------------------------------------------
    laserdisc_get_field_code - return raw field
    information read from the disc
-------------------------------------------------*/

UINT32 laserdisc_get_field_code(device_t *device, UINT32 code, UINT8 zero_if_squelched)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	int field = ldcore->fieldnum;

	/* return nothing if the video is off (external devices can't sense) */
	if (zero_if_squelched && ldcore->videosquelch)
		return 0;

	switch (code)
	{
		case LASERDISC_CODE_WHITE_FLAG:
			return ldcore->metadata[field].white;

		case LASERDISC_CODE_LINE16:
			return ldcore->metadata[field].line16;

		case LASERDISC_CODE_LINE17:
			return ldcore->metadata[field].line17;

		case LASERDISC_CODE_LINE18:
			return ldcore->metadata[field].line18;

		case LASERDISC_CODE_LINE1718:
			return ldcore->metadata[field].line1718;
	}

	return 0;
}



/***************************************************************************
    PLAYER-TO-CORE INTERFACES
***************************************************************************/

/*-------------------------------------------------
    ldcore_get_safe_token - return a token with
    type checking from a device
-------------------------------------------------*/

laserdisc_state *ldcore_get_safe_token(device_t *device)
{
	return get_safe_token(device);
}


/*-------------------------------------------------
    ldcore_set_audio_squelch - set the left/right
    audio squelch states
-------------------------------------------------*/

void ldcore_set_audio_squelch(laserdisc_state *ld, UINT8 squelchleft, UINT8 squelchright)
{
	update_audio(ld);
	ld->core->audiosquelch = (squelchleft ? 1 : 0) | (squelchright ? 2 : 0);
}


/*-------------------------------------------------
    ldcore_set_video_squelch - set the video
    squelch state
-------------------------------------------------*/

void ldcore_set_video_squelch(laserdisc_state *ld, UINT8 squelch)
{
	ld->core->videosquelch = squelch;
}


/*-------------------------------------------------
    ldcore_set_slider_speed - dynamically change
    the slider speed
-------------------------------------------------*/

void ldcore_set_slider_speed(laserdisc_state *ld, INT32 tracks_per_vsync)
{
	ldcore_data *ldcore = ld->core;
	attotime vsyncperiod = ld->screen->frame_period();

	update_slider_pos(ldcore, ld->device->machine().time());

	/* if 0, set the time to 0 */
	if (tracks_per_vsync == 0)
		ldcore->attospertrack = 0;

	/* positive values store positive times */
	else if (tracks_per_vsync > 0)
		ldcore->attospertrack = (vsyncperiod / tracks_per_vsync).as_attoseconds();

	/* negative values store negative times */
	else
		ldcore->attospertrack = -(vsyncperiod / -tracks_per_vsync).as_attoseconds();

	if (LOG_SLIDER)
		printf("Slider speed = %d\n", tracks_per_vsync);
}


/*-------------------------------------------------
    ldcore_advance_slider - advance the slider by
    a certain number of tracks
-------------------------------------------------*/

void ldcore_advance_slider(laserdisc_state *ld, INT32 numtracks)
{
	ldcore_data *ldcore = ld->core;

	update_slider_pos(ldcore, ld->device->machine().time());
	add_and_clamp_track(ldcore, numtracks);
	if (LOG_SLIDER)
		printf("Advance by %d\n", numtracks);
}


/*-------------------------------------------------
    ldcore_get_slider_position - get the current
    slider position
-------------------------------------------------*/

slider_position ldcore_get_slider_position(laserdisc_state *ld)
{
	ldcore_data *ldcore = ld->core;

	/* update the slider position first */
	update_slider_pos(ldcore, ld->device->machine().time());

	/* return the status */
	if (ldcore->curtrack == 1)
		return SLIDER_MINIMUM;
	else if (ldcore->curtrack < VIRTUAL_LEAD_IN_TRACKS)
		return SLIDER_VIRTUAL_LEADIN;
	else if (ldcore->curtrack < VIRTUAL_LEAD_IN_TRACKS + ldcore->chdtracks)
		return SLIDER_CHD;
	else if (ldcore->curtrack < VIRTUAL_LEAD_IN_TRACKS + MAX_TOTAL_TRACKS)
		return SLIDER_OUTSIDE_CHD;
	else if (ldcore->curtrack < ldcore->maxtrack - 1)
		return SLIDER_VIRTUAL_LEADOUT;
	else
		return SLIDER_MAXIMUM;
}



/***************************************************************************
    GENERIC HELPER FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    ldcore_generic_update - generically update in
    a way that works for most situations
-------------------------------------------------*/

INT32 ldcore_generic_update(laserdisc_state *ld, const vbi_metadata *vbi, int fieldnum, attotime curtime, ldplayer_state *newstate)
{
	INT32 advanceby = 0;
	int frame;

	/* start by assuming the state doesn't change */
	*newstate = ld->state;

	/* handle things based on the state */
	switch (ld->state.state)
	{
		case LDSTATE_EJECTING:
			/* when time expires, switch to the ejected state */
			if (curtime >= ld->state.endtime)
			    newstate->state = LDSTATE_EJECTED;
			break;

		case LDSTATE_EJECTED:
			/* do nothing */
			break;

		case LDSTATE_PARKED:
			/* do nothing */
			break;

		case LDSTATE_LOADING:
			/* when time expires, switch to the spinup state */
			if (curtime >= ld->state.endtime)
			    newstate->state = LDSTATE_SPINUP;
			advanceby = -GENERIC_SEARCH_SPEED;
			break;

		case LDSTATE_SPINUP:
			/* when time expires, switch to the playing state */
			if (curtime >= ld->state.endtime)
			    newstate->state = LDSTATE_PLAYING;
			advanceby = -GENERIC_SEARCH_SPEED;
			break;

		case LDSTATE_PAUSING:
			/* if he hit the start of a frame, switch to paused state */
			if (is_start_of_frame(vbi))
			{
			    newstate->state = LDSTATE_PAUSED;
			    newstate->param = fieldnum;
			}

			/* else advance until we hit it */
			else if (fieldnum == 1)
				advanceby = 1;
			break;

		case LDSTATE_PAUSED:
			/* if we paused on field 1, we must flip back and forth */
			if (ld->state.param == 1)
				advanceby = (fieldnum == 1) ? 1 : -1;
			break;

		case LDSTATE_PLAYING:
			/* if we hit the target frame, switch to the paused state */
			if (ld->state.param > 0 && is_start_of_frame(vbi) && frame_from_metadata(vbi) == ld->state.param)
			{
			    newstate->state = LDSTATE_PAUSED;
			    newstate->param = fieldnum;
			}

			/* otherwise after the second field of each frame */
			else if (fieldnum == 1)
				advanceby = 1;
			break;

		case LDSTATE_PLAYING_SLOW_REVERSE:
			/* after the second field of each frame, see if we need to advance */
			if (fieldnum == 1 && ++ld->state.substate > ld->state.param)
			{
				advanceby = -1;
				ld->state.substate = 0;
			}
			break;

		case LDSTATE_PLAYING_SLOW_FORWARD:
			/* after the second field of each frame, see if we need to advance */
			if (fieldnum == 1 && ++ld->state.substate > ld->state.param)
			{
				advanceby = 1;
				ld->state.substate = 0;
			}
			break;

		case LDSTATE_PLAYING_FAST_REVERSE:
			/* advance after the second field of each frame */
			if (fieldnum == 1)
				advanceby = -ld->state.param;
			break;

		case LDSTATE_PLAYING_FAST_FORWARD:
			/* advance after the second field of each frame */
			if (fieldnum == 1)
				advanceby = ld->state.param;
			break;

		case LDSTATE_SCANNING:
			/* advance after the second field of each frame */
			if (fieldnum == 1)
				advanceby = ld->state.param >> 8;

			/* after we run out of vsyncs, revert to the saved state */
			if (++ld->state.substate >= (ld->state.param & 0xff))
				*newstate = ld->savestate;
			break;

		case LDSTATE_STEPPING_REVERSE:
			/* wait for the first field of the frame and then leap backwards */
			if (is_start_of_frame(vbi))
			{
				advanceby = (fieldnum == 1) ? -1 : -2;
			    newstate->state = LDSTATE_PAUSING;
			}
			break;

		case LDSTATE_STEPPING_FORWARD:
			/* wait for the first field of the frame and then switch to pausing state */
			if (is_start_of_frame(vbi))
			    newstate->state = LDSTATE_PAUSING;
			break;

		case LDSTATE_SEEKING:
			/* if we're in the final state, look for a matching frame and pause there */
			frame = frame_from_metadata(vbi);
			if (ld->state.substate == 1 && is_start_of_frame(vbi) && frame == ld->state.param)
			{
			    newstate->state = LDSTATE_PAUSED;
			    newstate->param = fieldnum;
			}

			/* otherwise, if we got frame data from the VBI, update our seeking logic */
			else if (ld->state.substate == 0 && frame != FRAME_NOT_PRESENT)
			{
				INT32 delta = (ld->state.param - 2) - frame;

				/* if we're within a couple of frames, just play until we hit it */
				if (delta >= 0 && delta <= 2)
					ld->state.substate++;

				/* otherwise, compute the delta assuming 1:1 track to frame; this will correct eventually */
				else
				{
					if (delta < 0)
						delta--;
					advanceby = delta;
					advanceby = MIN(advanceby, GENERIC_SEARCH_SPEED);
					advanceby = MAX(advanceby, -GENERIC_SEARCH_SPEED);
				}
			}

			/* otherwise, keep advancing until we know what's up */
			else
			{
				if (fieldnum == 1)
					advanceby = 1;
			}
			break;
	}

	return advanceby;
}


/*-------------------------------------------------
    read_track_data - read and process data for
    a particular video track
-------------------------------------------------*/

static void read_track_data(laserdisc_state *ld)
{
	ldcore_data *ldcore = ld->core;
	UINT32 tracknum = ldcore->curtrack;
	UINT32 fieldnum = ldcore->fieldnum;
	vbi_metadata vbidata = { 0 };
	frame_data *frame;
	UINT32 vbiframe;
	UINT32 readhunk;
	INT32 chdtrack;

	/* compute the chdhunk number we are going to read */
	chdtrack = tracknum - 1 - VIRTUAL_LEAD_IN_TRACKS;
	chdtrack = MAX(chdtrack, 0);
	chdtrack = MIN(chdtrack, ldcore->chdtracks - 1);
	readhunk = chdtrack * 2 + fieldnum;

	/* cheat and look up the metadata we are about to retrieve */
	if (ldcore->vbidata != NULL)
		vbi_metadata_unpack(&vbidata, NULL, &ldcore->vbidata[readhunk * VBI_PACKED_BYTES]);

	/* if we're in the lead-in area, force the VBI data to be standard lead-in */
	if (tracknum - 1 < VIRTUAL_LEAD_IN_TRACKS)
	{
		vbidata.line16 = 0;
		vbidata.line17 = vbidata.line18 = vbidata.line1718 = VBI_CODE_LEADIN;
	}
//printf("track %5d.%d: %06X %06X %06X\n", tracknum, fieldnum, vbidata.line16, vbidata.line17, vbidata.line18);

	/* if we're about to read the first field in a frame, advance */
	frame = &ldcore->frame[ldcore->videoindex];
	if ((vbidata.line1718 & VBI_MASK_CAV_PICTURE) == VBI_CODE_CAV_PICTURE)
	{
		if (frame->numfields >= 2)
			ldcore->videoindex = (ldcore->videoindex + 1) % ARRAY_LENGTH(ldcore->frame);
		frame = &ldcore->frame[ldcore->videoindex];
		frame->numfields = 0;
	}

	/* if we're squelched, reset the frame counter */
	if (ldcore->videosquelch)
		frame->numfields = 0;

	/* remember the last field number */
	frame->lastfield = tracknum * 2 + fieldnum;

	/* set the video target information */
	ldcore->videotarget.alloc = NULL;
	ldcore->videotarget.base = BITMAP_ADDR16(frame->bitmap, fieldnum, 0);
	ldcore->videotarget.rowpixels = frame->bitmap->rowpixels * 2;
	ldcore->videotarget.width = frame->bitmap->width;
	ldcore->videotarget.height = frame->bitmap->height / 2;
	ldcore->videotarget.format = frame->bitmap->format;
	ldcore->videotarget.bpp = frame->bitmap->bpp;
	ldcore->videotarget.palette = frame->bitmap->palette;
	ldcore->videotarget.cliprect = frame->bitmap->cliprect;
	ldcore->avconfig.video = &ldcore->videotarget;

	/* set the audio target information */
	if (ldcore->audiobufin + ldcore->audiomaxsamples <= ldcore->audiobufsize)
	{
		/* if we can fit without wrapping, just read the data directly */
		ldcore->avconfig.audio[0] = &ldcore->audiobuffer[0][ldcore->audiobufin];
		ldcore->avconfig.audio[1] = &ldcore->audiobuffer[1][ldcore->audiobufin];
	}
	else
	{
		/* otherwise, read to the beginning of the buffer */
		ldcore->avconfig.audio[0] = &ldcore->audiobuffer[0][0];
		ldcore->avconfig.audio[1] = &ldcore->audiobuffer[1][0];
	}

	/* override if we're not decoding */
	ldcore->avconfig.maxsamples = ldcore->audiomaxsamples;
	ldcore->avconfig.actsamples = &ldcore->audiocursamples;
	ldcore->audiocursamples = 0;

	/* set the VBI data for the new field from our precomputed data */
	if (ldcore->vbidata != NULL)
		vbi_metadata_unpack(&ldcore->metadata[fieldnum], &vbiframe, &ldcore->vbidata[readhunk * VBI_PACKED_BYTES]);

	/* if we're in the lead-in area, force the VBI data to be standard lead-in */
	if (tracknum - 1 < VIRTUAL_LEAD_IN_TRACKS)
	{
		ldcore->metadata[fieldnum].line16 = 0;
		ldcore->metadata[fieldnum].line17 = ldcore->metadata[fieldnum].line18 = ldcore->metadata[fieldnum].line1718 = VBI_CODE_LEADIN;
	}

	/* configure the codec and then read */
	ldcore->readresult = CHDERR_FILE_NOT_FOUND;
	if (ldcore->disc != NULL && !ldcore->videosquelch)
	{
		ldcore->readresult = chd_codec_config(ldcore->disc, AV_CODEC_DECOMPRESS_CONFIG, &ldcore->avconfig);
		if (ldcore->readresult == CHDERR_NONE)
			ldcore->readresult = chd_read_async(ldcore->disc, readhunk, NULL);
	}
}


/*-------------------------------------------------
    process_track_data - process data from a
    track after it has been read
-------------------------------------------------*/

static void process_track_data(device_t *device)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;

	/* wait for the async operation to complete */
	if (ldcore->readresult == CHDERR_OPERATION_PENDING)
		ldcore->readresult = chd_async_complete(ldcore->disc);

	/* remove the video if we had an error */
	if (ldcore->readresult != CHDERR_NONE)
		ldcore->avconfig.video = NULL;

	/* count the field as read if we are successful */
	if (ldcore->avconfig.video != NULL)
		ldcore->frame[ldcore->videoindex].numfields++;

	/* render the display if present */
	if (ldcore->avconfig.video != NULL && ldcore->intf.overlay != NULL)
		(*ldcore->intf.overlay)(ld, ldcore->avconfig.video);

	/* pass the audio to the callback */
	if (ldcore->config.audio != NULL)
		(*ldcore->config.audio)(device, ldcore->samplerate, ldcore->audiocursamples, ldcore->avconfig.audio[0], ldcore->avconfig.audio[1]);

	/* shift audio data if we read it into the beginning of the buffer */
	if (ldcore->audiocursamples != 0 && ldcore->audiobufin != 0)
	{
		int chnum;

		/* iterate over channels */
		for (chnum = 0; chnum < 2; chnum++)
			if (ldcore->avconfig.audio[chnum] == &ldcore->audiobuffer[chnum][0])
			{
				int samplesleft;

				/* move data to the end */
				samplesleft = ldcore->audiobufsize - ldcore->audiobufin;
				samplesleft = MIN(samplesleft, ldcore->audiocursamples);
				memmove(&ldcore->audiobuffer[chnum][ldcore->audiobufin], &ldcore->audiobuffer[chnum][0], samplesleft * 2);

				/* shift data at the beginning */
				if (samplesleft < ldcore->audiocursamples)
					memmove(&ldcore->audiobuffer[chnum][0], &ldcore->audiobuffer[chnum][samplesleft], (ldcore->audiocursamples - samplesleft) * 2);
			}
	}

	/* update the input buffer pointer */
	ldcore->audiobufin = (ldcore->audiobufin + ldcore->audiocursamples) % ldcore->audiobufsize;
}


/*-------------------------------------------------
    laserdisc_sound_start - custom audio start
    for laserdiscs
-------------------------------------------------*/

static DEVICE_START( laserdisc_sound )
{
	sound_token *token = (sound_token *)downcast<legacy_device_base *>(device)->token();
	token->stream = device->machine().sound().stream_alloc(*device, 0, 2, 48000, token, custom_stream_callback);
	token->ld = NULL;
}


/*-------------------------------------------------
    laserdisc_sound_get_info - information
    callback for laserdisc audio
-------------------------------------------------*/

DEVICE_GET_INFO( laserdisc_sound )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:					info->i = sizeof(sound_token);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:							info->start = DEVICE_START_NAME(laserdisc_sound);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Laserdisc Analog");			break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);						break;
	}
}


/*-------------------------------------------------
    custom_stream_callback - audio streamer
    for laserdiscs
-------------------------------------------------*/

static STREAM_UPDATE( custom_stream_callback )
{
	sound_token *token = (sound_token *)param;
	laserdisc_state *ld = token->ld;
	ldcore_data *ldcore = ld->core;
	stream_sample_t *dst0 = outputs[0];
	stream_sample_t *dst1 = outputs[1];
	INT16 leftand, rightand;
	int samples_avail = 0;

	/* compute AND values based on the squelch */
	leftand = (ldcore->audiosquelch & 1) ? 0x0000 : 0xffff;
	rightand = (ldcore->audiosquelch & 2) ? 0x0000 : 0xffff;

	/* see if we have enough samples to fill the buffer; if not, drop out */
	if (ld != NULL)
	{
		samples_avail = ldcore->audiobufin - ldcore->audiobufout;
		if (samples_avail < 0)
			samples_avail += ldcore->audiobufsize;
	}

	/* if no attached ld, just clear the buffers */
	if (samples_avail < samples)
	{
		memset(dst0, 0, samples * sizeof(dst0[0]));
		memset(dst1, 0, samples * sizeof(dst1[0]));
	}

	/* otherwise, stream from our buffer */
	else
	{
		INT16 *buffer0 = ldcore->audiobuffer[0];
		INT16 *buffer1 = ldcore->audiobuffer[1];
		int sampout = ldcore->audiobufout;

		/* copy samples, clearing behind us as we go */
		while (sampout != ldcore->audiobufin && samples-- > 0)
		{
			*dst0++ = buffer0[sampout] & leftand;
			*dst1++ = buffer1[sampout] & rightand;
			buffer0[sampout] = 0;
			buffer1[sampout] = 0;
			sampout++;
			if (sampout >= ldcore->audiobufsize)
				sampout = 0;
		}
		ldcore->audiobufout = sampout;

		/* clear out the rest of the buffer */
		if (samples > 0)
		{
			int sampout = (ldcore->audiobufout == 0) ? ldcore->audiobufsize - 1 : ldcore->audiobufout - 1;
			stream_sample_t fill0 = buffer0[sampout] & leftand;
			stream_sample_t fill1 = buffer1[sampout] & rightand;

			while (samples-- > 0)
			{
				*dst0++ = fill0;
				*dst1++ = fill1;
			}
		}
	}
}



/***************************************************************************
    CONFIG SETTINGS ACCESS
***************************************************************************/

/*-------------------------------------------------
    configuration_load - read and apply data from
    the configuration file
-------------------------------------------------*/

static void configuration_load(device_t *device, int config_type, xml_data_node *parentnode)
{
	xml_data_node *overnode;
	xml_data_node *ldnode;

	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	/* might not have any data */
	if (parentnode == NULL)
		return;

	/* iterate over overlay nodes */
	for (ldnode = xml_get_sibling(parentnode->child, "device"); ldnode != NULL; ldnode = xml_get_sibling(ldnode->next, "device"))
	{
		const char *devtag = xml_get_attribute_string(ldnode, "tag", "");
		if (strcmp(devtag,device->tag()) == 0)
		{
			laserdisc_state *ld = get_safe_token(device);
			ldcore_data *ldcore = ld->core;

			/* handle the overlay node */
			overnode = xml_get_sibling(ldnode->child, "overlay");
			if (overnode != NULL)
			{
				/* fetch positioning controls */
				ldcore->config.overposx = xml_get_attribute_float(overnode, "hoffset", ldcore->config.overposx);
				ldcore->config.overscalex = xml_get_attribute_float(overnode, "hstretch", ldcore->config.overscalex);
				ldcore->config.overposy = xml_get_attribute_float(overnode, "voffset", ldcore->config.overposy);
				ldcore->config.overscaley = xml_get_attribute_float(overnode, "vstretch", ldcore->config.overscaley);
			}
		}
	}
}


/*-------------------------------------------------
    configuration_save - save data to the
    configuration file
-------------------------------------------------*/

static void configuration_save(device_t *device, int config_type, xml_data_node *parentnode)
{
	/* we only care about game files */
	if (config_type != CONFIG_TYPE_GAME)
		return;

	laserdisc_config *origconfig = (laserdisc_config *)downcast<const legacy_device_base *>(device)->inline_config();
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	xml_data_node *overnode;
	xml_data_node *ldnode;

	/* create a node */
	ldnode = xml_add_child(parentnode, "device", NULL);
	if (ldnode != NULL)
	{
		int changed = FALSE;

		/* output the basics */
		xml_set_attribute(ldnode, "tag", device->tag());

		/* add an overlay node */
		overnode = xml_add_child(ldnode, "overlay", NULL);
		if (overnode != NULL)
		{
			/* output the positioning controls */
			if (ldcore->config.overposx != origconfig->overposx)
			{
				xml_set_attribute_float(overnode, "hoffset", ldcore->config.overposx);
				changed = TRUE;
			}

			if (ldcore->config.overscalex != origconfig->overscalex)
			{
				xml_set_attribute_float(overnode, "hstretch", ldcore->config.overscalex);
				changed = TRUE;
			}

			if (ldcore->config.overposy != origconfig->overposy)
			{
				xml_set_attribute_float(overnode, "voffset", ldcore->config.overposy);
				changed = TRUE;
			}

			if (ldcore->config.overscaley != origconfig->overscaley)
			{
				xml_set_attribute_float(overnode, "vstretch", ldcore->config.overscaley);
				changed = TRUE;
			}
		}

		/* if nothing changed, kill the node */
		if (!changed)
			xml_delete_node(ldnode);
	}
}



/***************************************************************************
    VIDEO INTERFACE
***************************************************************************/

/*-------------------------------------------------
    laserdisc_video_enable - enable/disable the
    video
-------------------------------------------------*/

void laserdisc_video_enable(device_t *device, int enable)
{
	laserdisc_state *ld = get_safe_token(device);
	ld->core->videoenable = enable;
}


/*-------------------------------------------------
    laserdisc_video_enable - enable/disable the
    video
-------------------------------------------------*/

void laserdisc_overlay_enable(device_t *device, int enable)
{
	laserdisc_state *ld = get_safe_token(device);
	ld->core->overenable = enable;
}


/*-------------------------------------------------
    video update callback
-------------------------------------------------*/

SCREEN_UPDATE( laserdisc )
{
	device_t *laserdisc = screen->machine().device("laserdisc"); // TODO: allow more than one laserdisc
	if (laserdisc != NULL)
	{
		const rectangle &visarea = screen->visible_area();
		laserdisc_state *ld = (laserdisc_state *)downcast<legacy_device_base *>(laserdisc)->token();
		ldcore_data *ldcore = ld->core;
		bitmap_t *overbitmap = ldcore->overbitmap[ldcore->overindex];
		bitmap_t *vidbitmap = NULL;

		/* handle the overlay if present */
		if (overbitmap != NULL && ldcore->config.overupdate != NULL)
		{
			rectangle clip = *cliprect;

			/* scale the cliprect to the overlay size and then call the update callback */
			clip.min_x = ldcore->config.overclip.min_x;
			clip.max_x = ldcore->config.overclip.max_x;
			clip.min_y = cliprect->min_y * overbitmap->height / bitmap->height;
			if (cliprect->min_y == visarea.min_y)
				clip.min_y = MIN(clip.min_y, ldcore->config.overclip.min_y);
			clip.max_y = (cliprect->max_y + 1) * overbitmap->height / bitmap->height - 1;
			(*ldcore->config.overupdate)(screen, overbitmap, &clip);
		}

		/* if this is the last update, do the rendering */
		if (cliprect->max_y == visarea.max_y)
		{
			/* update the texture with the overlay contents */
			if (overbitmap != NULL)
			{
				if (overbitmap->format == BITMAP_FORMAT_INDEXED16)
					ldcore->overtex->set_bitmap(overbitmap, &ldcore->config.overclip, TEXFORMAT_PALETTEA16, laserdisc->machine().palette);
				else if (overbitmap->format == BITMAP_FORMAT_RGB32)
					ldcore->overtex->set_bitmap(overbitmap, &ldcore->config.overclip, TEXFORMAT_ARGB32);
			}

			/* get the laserdisc video */
			laserdisc_get_video(laserdisc, &vidbitmap);
			if (vidbitmap != NULL)
				ldcore->videotex->set_bitmap(vidbitmap, NULL, TEXFORMAT_YUY16, ldcore->videopalette);

			/* reset the screen contents */
			screen->container().empty();

			/* add the video texture */
			if (ldcore->videoenable)
				screen->container().add_quad(0.0f, 0.0f, 1.0f, 1.0f, MAKE_ARGB(0xff,0xff,0xff,0xff), ldcore->videotex, PRIMFLAG_BLENDMODE(BLENDMODE_NONE) | PRIMFLAG_SCREENTEX(1));

			/* add the overlay */
			if (ldcore->overenable && overbitmap != NULL)
			{
				float x0 = 0.5f - 0.5f * ldcore->config.overscalex + ldcore->config.overposx;
				float y0 = 0.5f - 0.5f * ldcore->config.overscaley + ldcore->config.overposy;
				float x1 = x0 + ldcore->config.overscalex;
				float y1 = y0 + ldcore->config.overscaley;
				screen->container().add_quad(x0, y0, x1, y1, MAKE_ARGB(0xff,0xff,0xff,0xff), ldcore->overtex, PRIMFLAG_BLENDMODE(BLENDMODE_ALPHA) | PRIMFLAG_SCREENTEX(1));
			}

			/* swap to the next bitmap */
			ldcore->overindex = (ldcore->overindex + 1) % ARRAY_LENGTH(ldcore->overbitmap);
		}
	}

	return 0;
}



/***************************************************************************
    CONFIGURATION
***************************************************************************/

/*-------------------------------------------------
    laserdisc_get_config - return a copy of the
    current live configuration settings
-------------------------------------------------*/

void laserdisc_get_config(device_t *device, laserdisc_config *config)
{
	laserdisc_state *ld = get_safe_token(device);
	*config = ld->core->config;
}


/*-------------------------------------------------
    laserdisc_get_config - change the current live
    configuration settings
-------------------------------------------------*/

void laserdisc_set_config(device_t *device, const laserdisc_config *config)
{
	laserdisc_state *ld = get_safe_token(device);
	ld->core->config = *config;
}



/***************************************************************************
    INITIALIZATION
***************************************************************************/

/*-------------------------------------------------
    init_disc - initialize the state of the
    CHD disc
-------------------------------------------------*/

static void init_disc(device_t *device)
{
	const laserdisc_config *config = (const laserdisc_config *)downcast<const legacy_device_base *>(device)->inline_config();
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	chd_error err;

	/* get a handle to the disc to play */
	if (config->getdisc != NULL)
		ldcore->disc = (*config->getdisc)(device);
	else
		ldcore->disc = get_disk_handle(device->machine(), device->tag());

	/* set default parameters */
	ldcore->width = 720;
	ldcore->height = 240;
	ldcore->fps_times_1million = 59940000;
	ldcore->samplerate = 48000;

	/* get the disc metadata and extract the ld */
	ldcore->chdtracks = 0;
	ldcore->maxtrack = VIRTUAL_LEAD_IN_TRACKS + MAX_TOTAL_TRACKS + VIRTUAL_LEAD_OUT_TRACKS;
	if (ldcore->disc != NULL)
	{
		UINT32 totalhunks = chd_get_header(ldcore->disc)->totalhunks;
		int fps, fpsfrac, interlaced, channels;
		char metadata[256];
		UINT32 vbilength;

		/* require the A/V codec */
		if (chd_get_header(ldcore->disc)->compression != CHDCOMPRESSION_AV)
			fatalerror("Laserdisc video must be compressed with the A/V codec!");

		/* read the metadata */
		err = chd_get_metadata(ldcore->disc, AV_METADATA_TAG, 0, metadata, sizeof(metadata), NULL, NULL, NULL);
		if (err != CHDERR_NONE)
			fatalerror("Non-A/V CHD file specified");

		/* extract the metadata */
		if (sscanf(metadata, AV_METADATA_FORMAT, &fps, &fpsfrac, &ldcore->width, &ldcore->height, &interlaced, &channels, &ldcore->samplerate) != 7)
			fatalerror("Invalid metadata in CHD file");
		else
			ldcore->fps_times_1million = fps * 1000000 + fpsfrac;

		/* require interlaced video */
		if (!interlaced)
			fatalerror("Laserdisc video must be interlaced!");

		/* determine the maximum track and allocate a frame buffer */
		ldcore->chdtracks = totalhunks / 2;

		/* allocate memory for the precomputed per-frame metadata */
		ldcore->vbidata = auto_alloc_array(device->machine(), UINT8, totalhunks * VBI_PACKED_BYTES);
		err = chd_get_metadata(ldcore->disc, AV_LD_METADATA_TAG, 0, ldcore->vbidata, totalhunks * VBI_PACKED_BYTES, &vbilength, NULL, NULL);
		if (err != CHDERR_NONE || vbilength != totalhunks * VBI_PACKED_BYTES)
			fatalerror("Precomputed VBI metadata missing or incorrect size");
	}
	ldcore->maxtrack = MAX(ldcore->maxtrack, VIRTUAL_LEAD_IN_TRACKS + VIRTUAL_LEAD_OUT_TRACKS + ldcore->chdtracks);
}


/*-------------------------------------------------
    init_video - initialize the state of the
    video rendering
-------------------------------------------------*/

static void init_video(device_t *device)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;
	int index;

	/* register for VBLANK callbacks */
	ld->screen->register_vblank_callback(vblank_state_delegate(FUNC(vblank_state_changed), device));

	/* allocate video frames */
	for (index = 0; index < ARRAY_LENGTH(ldcore->frame); index++)
	{
		frame_data *frame = &ldcore->frame[index];

		/* first allocate a YUY16 bitmap at 2x the height */
		frame->bitmap = auto_alloc(device->machine(), bitmap_t(ldcore->width, ldcore->height * 2, BITMAP_FORMAT_YUY16));
		fillbitmap_yuy16(frame->bitmap, 40, 109, 240);

		/* make a copy of the bitmap that clips out the VBI and horizontal blanking areas */
		frame->visbitmap = auto_alloc(device->machine(), bitmap_t(BITMAP_ADDR16(frame->bitmap, 44, frame->bitmap->width * 8 / 720),
																frame->bitmap->width - 2 * frame->bitmap->width * 8 / 720,
																frame->bitmap->height - 44,
																frame->bitmap->rowpixels, frame->bitmap->format));
	}

	/* allocate an empty frame of the same size */
	ldcore->emptyframe = auto_bitmap_alloc(device->machine(), ldcore->width, ldcore->height * 2, BITMAP_FORMAT_YUY16);
	fillbitmap_yuy16(ldcore->emptyframe, 0, 128, 128);

	/* allocate texture for rendering */
	ldcore->videoenable = TRUE;
	ldcore->videotex = device->machine().render().texture_alloc();
	if (ldcore->videotex == NULL)
		fatalerror("Out of memory allocating video texture");

	/* allocate palette for applying brightness/contrast/gamma */
	ldcore->videopalette = palette_alloc(256, 1);
	if (ldcore->videopalette == NULL)
		fatalerror("Out of memory allocating video palette");
	for (index = 0; index < 256; index++)
		palette_entry_set_color(ldcore->videopalette, index, MAKE_RGB(index, index, index));

	/* allocate overlay */
	if (ldcore->config.overwidth > 0 && ldcore->config.overheight > 0 && ldcore->config.overupdate != NULL)
	{
		ldcore->overenable = TRUE;
		ldcore->overbitmap[0] = auto_bitmap_alloc(device->machine(), ldcore->config.overwidth, ldcore->config.overheight, (bitmap_format)ldcore->config.overformat);
		ldcore->overbitmap[1] = auto_bitmap_alloc(device->machine(), ldcore->config.overwidth, ldcore->config.overheight, (bitmap_format)ldcore->config.overformat);
		ldcore->overtex = device->machine().render().texture_alloc();
		if (ldcore->overtex == NULL)
			fatalerror("Out of memory allocating overlay texture");
	}
}


/*-------------------------------------------------
    init_audio - initialize the state of the
    audio rendering
-------------------------------------------------*/

static void init_audio(device_t *device)
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;

	/* find the custom audio */
	ldcore->audiocustom = device->machine().device(ldcore->config.sound);

	/* allocate audio buffers */
	ldcore->audiomaxsamples = ((UINT64)ldcore->samplerate * 1000000 + ldcore->fps_times_1million - 1) / ldcore->fps_times_1million;
	ldcore->audiobufsize = ldcore->audiomaxsamples * 4;
	ldcore->audiobuffer[0] = auto_alloc_array(device->machine(), INT16, ldcore->audiobufsize);
	ldcore->audiobuffer[1] = auto_alloc_array(device->machine(), INT16, ldcore->audiobufsize);
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( laserdisc )
{
	const laserdisc_config *config = (const laserdisc_config *)downcast<const legacy_device_base *>(device)->inline_config();
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore;

	/* ensure that our screen is started first */
	ld->screen = downcast<screen_device *>(device->machine().device(config->screen));
	assert(ld->screen != NULL);
	if (!ld->screen->started())
		throw device_missing_dependencies();

	/* save a copy of the device pointer */
	ld->device = device;

	/* allocate memory for the core state */
	ld->core = auto_alloc_clear(device->machine(), ldcore_data);
	ldcore = ld->core;

	if (device->type() == PIONEER_PR8210)
		ldcore->intf = pr8210_interface;
	else if (device->type() == SIMUTREK_SPECIAL)
		ldcore->intf = simutrek_interface;
	else if (device->type() == PIONEER_LDV1000)
		ldcore->intf = ldv1000_interface;
	else if (device->type() == PHILLIPS_22VP931)
		ldcore->intf = vp931_interface;

	ld->player = (ldplayer_data *)auto_alloc_array_clear(device->machine(), UINT8, ldcore->intf.statesize);

	/* copy config data to the live state */
	ldcore->config = *config;
	if (ldcore->config.overclip.max_x == ldcore->config.overclip.min_x || ldcore->config.overclip.max_y == ldcore->config.overclip.min_y)
	{
		ldcore->config.overclip.min_x = ldcore->config.overclip.min_y = 0;
		ldcore->config.overclip.max_x = ldcore->config.overwidth - 1;
		ldcore->config.overclip.max_y = ldcore->config.overheight - 1;
	}
	if (ldcore->config.overscalex == 0)
		ldcore->config.overscalex = 1.0f;
	if (ldcore->config.overscaley == 0)
		ldcore->config.overscaley = 1.0f;

	/* initialize the various pieces */
	init_disc(device);
	init_video(device);
	init_audio(device);

	/* register callbacks */
	config_register(device->machine(), "laserdisc", config_saveload_delegate(FUNC(configuration_load), device), config_saveload_delegate(FUNC(configuration_save), device));
}


/*-------------------------------------------------
    device exit callback
-------------------------------------------------*/

static DEVICE_STOP( laserdisc )
{
	laserdisc_state *ld = get_safe_token(device);
	ldcore_data *ldcore = ld->core;

	/* make sure all async operations have completed */
	if (ldcore->disc != NULL)
		chd_async_complete(ldcore->disc);

	/* free any textures and palettes */
	device->machine().render().texture_free(ldcore->videotex);
	if (ldcore->videopalette != NULL)
		palette_deref(ldcore->videopalette);
	device->machine().render().texture_free(ldcore->overtex);
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( laserdisc )
{
	laserdisc_state *ld = get_safe_token(device);
	attotime curtime = device->machine().time();
	ldcore_data *ldcore = ld->core;
	int line;

	/* attempt to wire up the audio */
	if (ldcore->audiocustom != NULL)
	{
		sound_token *token = (sound_token *)downcast<legacy_device_base *>(ldcore->audiocustom)->token();
		token->ld = ld;
		token->stream->set_sample_rate(ldcore->samplerate);
	}

	/* set up the general ld */
	ldcore->audiosquelch = 3;
	ldcore->videosquelch = 1;
	ldcore->fieldnum = 0;
	ldcore->curtrack = 1;
	ldcore->attospertrack = 0;
	ldcore->sliderupdate = curtime;

	/* reset the I/O lines */
	for (line = 0; line < LASERDISC_INPUT_LINES; line++)
		ldcore->linein[line] = CLEAR_LINE;
	for (line = 0; line < LASERDISC_OUTPUT_LINES; line++)
		ldcore->lineout[line] = CLEAR_LINE;

	/* call the initialization */
	if (ldcore->intf.init != NULL)
		(*ldcore->intf.init)(ld);
}


/*-------------------------------------------------
    device set info callback
-------------------------------------------------*/

int laserdisc_get_type(device_t *device)
{
	if (device->type() == PIONEER_PR7820)
		return LASERDISC_TYPE_PIONEER_PR7820;
	if (device->type() == PIONEER_PR8210)
		return LASERDISC_TYPE_PIONEER_PR8210;
	if (device->type() == SIMUTREK_SPECIAL)
		return LASERDISC_TYPE_SIMUTREK_SPECIAL;
	if (device->type() == PIONEER_LDV1000)
		return LASERDISC_TYPE_PIONEER_LDV1000;
	if (device->type() == PHILLIPS_22VP931)
		return LASERDISC_TYPE_PHILLIPS_22VP931;
	if (device->type() == PHILLIPS_22VP932)
		return LASERDISC_TYPE_PHILLIPS_22VP932;
	if (device->type() == SONY_LDP1450)
		return LASERDISC_TYPE_SONY_LDP1450;
	return LASERDISC_TYPE_UNKNOWN;
}

void laserdisc_set_type(device_t *device, int type)
{
	// this is no longer supported
}


DEVICE_GET_INFO( laserdisc )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(laserdisc_state);					break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(laserdisc_config);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(laserdisc); 		break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(laserdisc);			break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(laserdisc);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Laserdisc Player");				break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");								break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);							break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}

DEVICE_GET_INFO( unkldplay )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = NULL; return;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = NULL; return;
		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s,"Unknown Laserdisc Player"); return;
		case DEVINFO_STR_SHORTNAME:				strcpy(info->s,"unkldplay"); return;
	}

	DEVICE_GET_INFO_CALL( laserdisc );
}

DEVICE_GET_INFO( pioneer_pr7820 )
{
	DEVICE_GET_INFO_CALL( unkldplay );
}

DEVICE_GET_INFO( pioneer_pr8210 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = pr8210_interface.romregion; return;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = pr8210_interface.machine_config; return;

		/* --- the following bits of info are returned as NULL-terminated strings -- */
		case DEVINFO_STR_NAME:					strcpy(info->s, pr8210_interface.name); return;
		case DEVINFO_STR_SHORTNAME:				strcpy(info->s, pr8210_interface.shortname); return;
	}

	DEVICE_GET_INFO_CALL( laserdisc );
}

DEVICE_GET_INFO( simutrek_special )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = simutrek_interface.romregion; return;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = simutrek_interface.machine_config; return;

		/* --- the following bits of info are returned as NULL-terminated strings -- */
		case DEVINFO_STR_NAME:					strcpy(info->s, simutrek_interface.name); return;
		case DEVINFO_STR_SHORTNAME:				strcpy(info->s, simutrek_interface.shortname); return;
	}

	DEVICE_GET_INFO_CALL( laserdisc );
}

DEVICE_GET_INFO( pioneer_ldv1000 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = ldv1000_interface.romregion; return;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = ldv1000_interface.machine_config; return;

		/* --- the following bits of info are returned as NULL-terminated strings -- */
		case DEVINFO_STR_NAME:					strcpy(info->s, ldv1000_interface.name); return;
		case DEVINFO_STR_SHORTNAME:				strcpy(info->s, ldv1000_interface.shortname); return;
	}

	DEVICE_GET_INFO_CALL( laserdisc );
}

DEVICE_GET_INFO( phillips_22vp931 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers --- */
		case DEVINFO_PTR_ROM_REGION:			info->romregion = vp931_interface.romregion; return;
		case DEVINFO_PTR_MACHINE_CONFIG:		info->machine_config = vp931_interface.machine_config; return;

		/* --- the following bits of info are returned as NULL-terminated strings -- */
		case DEVINFO_STR_NAME:					strcpy(info->s, vp931_interface.name); return;
		case DEVINFO_STR_SHORTNAME:				strcpy(info->s, vp931_interface.shortname); return;
	}

	DEVICE_GET_INFO_CALL( laserdisc );
}

DEVICE_GET_INFO( phillips_22vp932 )
{
	DEVICE_GET_INFO_CALL( unkldplay );
}

DEVICE_GET_INFO( sony_ldp1450 )
{
	DEVICE_GET_INFO_CALL( unkldplay );
}

DEFINE_LEGACY_DEVICE(PIONEER_PR7820,pioneer_pr7820);
DEFINE_LEGACY_DEVICE(PIONEER_PR8210,pioneer_pr8210);
DEFINE_LEGACY_DEVICE(SIMUTREK_SPECIAL,simutrek_special);
DEFINE_LEGACY_DEVICE(PIONEER_LDV1000,pioneer_ldv1000);
DEFINE_LEGACY_DEVICE(PHILLIPS_22VP931,phillips_22vp931);
DEFINE_LEGACY_DEVICE(PHILLIPS_22VP932,phillips_22vp932);
DEFINE_LEGACY_DEVICE(SONY_LDP1450,sony_ldp1450);
DEFINE_LEGACY_SOUND_DEVICE(LASERDISC_SOUND, laserdisc_sound);
