/*
 *  Pulsar sound routines
 *
 *  TODO: change heart rate based on bit 7 of Port 1
 *
 */

#include "emu.h"
#include "sound/samples.h"
#include "includes/vicdual.h"


/* output port 0x01 definitions - sound effect drive outputs */
#define OUT_PORT_1_CLANG		0x01
#define OUT_PORT_1_KEY			0x02
#define OUT_PORT_1_ALIENHIT		0x04
#define OUT_PORT_1_PHIT			0x08
#define OUT_PORT_1_ASHOOT		0x10
#define OUT_PORT_1_PSHOOT		0x20
#define OUT_PORT_1_BONUS		0x40
#define OUT_PORT_1_HBEAT_RATE	0x80	/* currently not used */

/* output port 0x02 definitions - sound effect drive outputs */
#define OUT_PORT_2_SIZZLE		0x01
#define OUT_PORT_2_GATE			0x02
#define OUT_PORT_2_BIRTH		0x04
#define OUT_PORT_2_HBEAT		0x08
#define OUT_PORT_2_MOVMAZE		0x10


#define PLAY(samp,id,loop)           samp->start( id, id, loop )
#define STOP(samp,id)                samp->stop( id )


/* sample file names */
static const char *const pulsar_sample_names[] =
{
	"*pulsar",
	"clang",
	"key",
	"alienhit",
	"phit",
	"ashoot",
	"pshoot",
	"bonus",
	"sizzle",
	"gate",
	"birth",
	"hbeat",
	"movmaze",
	0
};


static const samples_interface pulsar_samples_interface =
{
	12,
	pulsar_sample_names
};


MACHINE_CONFIG_FRAGMENT( pulsar_audio )
	MCFG_SAMPLES_ADD("samples", pulsar_samples_interface)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.5)
MACHINE_CONFIG_END


/* sample IDs - must match sample file name table above */
enum
{
	SND_CLANG = 0,
	SND_KEY,
	SND_ALIENHIT,
	SND_PHIT,
	SND_ASHOOT,
	SND_PSHOOT,
	SND_BONUS,
	SND_SIZZLE,
	SND_GATE,
	SND_BIRTH,
	SND_HBEAT,
	SND_MOVMAZE
};


static int port1State = 0;

WRITE8_HANDLER( pulsar_audio_1_w )
{
	samples_device *samples = space->machine().device<samples_device>("samples");
	int bitsChanged;
	//int bitsGoneHigh;
	int bitsGoneLow;


	bitsChanged  = port1State ^ data;
	//bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port1State = data;

	if ( bitsGoneLow & OUT_PORT_1_CLANG )
	{
		PLAY( samples, SND_CLANG, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_KEY )
	{
		PLAY( samples, SND_KEY, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_ALIENHIT )
	{
		PLAY( samples, SND_ALIENHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_PHIT )
	{
		PLAY( samples, SND_PHIT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_ASHOOT )
	{
		PLAY( samples, SND_ASHOOT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_PSHOOT )
	{
		PLAY( samples, SND_PSHOOT, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_1_BONUS )
	{
		PLAY( samples, SND_BONUS, 0 );
	}
}


WRITE8_HANDLER( pulsar_audio_2_w )
{
	samples_device *samples = space->machine().device<samples_device>("samples");
	static int port2State = 0;
	int bitsChanged;
	int bitsGoneHigh;
	int bitsGoneLow;


	bitsChanged  = port2State ^ data;
	bitsGoneHigh = bitsChanged & data;
	bitsGoneLow  = bitsChanged & ~data;

	port2State = data;

	if ( bitsGoneLow & OUT_PORT_2_SIZZLE )
	{
		PLAY( samples, SND_SIZZLE, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_GATE )
	{
		samples->start(SND_CLANG, SND_GATE);
	}
	if ( bitsGoneHigh & OUT_PORT_2_GATE )
	{
		STOP( samples, SND_CLANG );
	}

	if ( bitsGoneLow & OUT_PORT_2_BIRTH )
	{
		PLAY( samples, SND_BIRTH, 0 );
	}

	if ( bitsGoneLow & OUT_PORT_2_HBEAT )
	{
		PLAY( samples, SND_HBEAT, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_2_HBEAT )
	{
		STOP( samples, SND_HBEAT );
	}

	if ( bitsGoneLow & OUT_PORT_2_MOVMAZE )
	{
		PLAY( samples, SND_MOVMAZE, 1 );
	}
	if ( bitsGoneHigh & OUT_PORT_2_MOVMAZE )
	{
		STOP( samples, SND_MOVMAZE );
	}
}
