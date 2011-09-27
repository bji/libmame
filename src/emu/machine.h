/***************************************************************************

    machine.h

    Controls execution of the core MAME system.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __MACHINE_H__
#define __MACHINE_H__

#include <time.h>



//**************************************************************************
//  CONSTANTS
//**************************************************************************

const int MAX_GFX_ELEMENTS = 32;


// machine phases
enum machine_phase
{
	MACHINE_PHASE_PREINIT,
	MACHINE_PHASE_INIT,
	MACHINE_PHASE_RESET,
	MACHINE_PHASE_RUNNING,
	MACHINE_PHASE_EXIT
};


// notification callback types
enum machine_notification
{
    MACHINE_NOTIFY_STARTUP,
	MACHINE_NOTIFY_FRAME,
	MACHINE_NOTIFY_RESET,
	MACHINE_NOTIFY_PAUSE,
	MACHINE_NOTIFY_RESUME,
	MACHINE_NOTIFY_EXIT,
	MACHINE_NOTIFY_COUNT
};


enum machine_init_phase
{
    STARTUP_PHASE_PREPARING,
    STARTUP_PHASE_LOADING_ROMS,
    STARTUP_PHASE_INITIALIZING_STATE
};


// output channels
enum output_channel
{
    OUTPUT_CHANNEL_ERROR,
    OUTPUT_CHANNEL_WARNING,
    OUTPUT_CHANNEL_INFO,
    OUTPUT_CHANNEL_DEBUG,
    OUTPUT_CHANNEL_VERBOSE,
    OUTPUT_CHANNEL_LOG,
    OUTPUT_CHANNEL_COUNT
};


// debug flags
const int DEBUG_FLAG_ENABLED		= 0x00000001;		// debugging is enabled
const int DEBUG_FLAG_CALL_HOOK		= 0x00000002;		// CPU cores must call instruction hook
const int DEBUG_FLAG_WPR_PROGRAM	= 0x00000010;		// watchpoints are enabled for PROGRAM memory reads
const int DEBUG_FLAG_WPR_DATA		= 0x00000020;		// watchpoints are enabled for DATA memory reads
const int DEBUG_FLAG_WPR_IO			= 0x00000040;		// watchpoints are enabled for IO memory reads
const int DEBUG_FLAG_WPW_PROGRAM	= 0x00000100;		// watchpoints are enabled for PROGRAM memory writes
const int DEBUG_FLAG_WPW_DATA		= 0x00000200;		// watchpoints are enabled for DATA memory writes
const int DEBUG_FLAG_WPW_IO			= 0x00000400;		// watchpoints are enabled for IO memory writes
const int DEBUG_FLAG_OSD_ENABLED	= 0x00001000;		// The OSD debugger is enabled



//**************************************************************************
//  MACROS
//**************************************************************************

// macros to wrap legacy callbacks
#define MACHINE_START_NAME(name)	machine_start_##name
#define MACHINE_START(name)			void MACHINE_START_NAME(name)(running_machine &machine)
#define MACHINE_START_CALL(name)	MACHINE_START_NAME(name)(machine)

#define MACHINE_RESET_NAME(name)	machine_reset_##name
#define MACHINE_RESET(name)			void MACHINE_RESET_NAME(name)(running_machine &machine)
#define MACHINE_RESET_CALL(name)	MACHINE_RESET_NAME(name)(machine)

#define SOUND_START_NAME(name)		sound_start_##name
#define SOUND_START(name)			void SOUND_START_NAME(name)(running_machine &machine)
#define SOUND_START_CALL(name)		SOUND_START_NAME(name)(machine)

#define SOUND_RESET_NAME(name)		sound_reset_##name
#define SOUND_RESET(name)			void SOUND_RESET_NAME(name)(running_machine &machine)
#define SOUND_RESET_CALL(name)		SOUND_RESET_NAME(name)(machine)

#define VIDEO_START_NAME(name)		video_start_##name
#define VIDEO_START(name)			void VIDEO_START_NAME(name)(running_machine &machine)
#define VIDEO_START_CALL(name)		VIDEO_START_NAME(name)(machine)

#define VIDEO_RESET_NAME(name)		video_reset_##name
#define VIDEO_RESET(name)			void VIDEO_RESET_NAME(name)(running_machine &machine)
#define VIDEO_RESET_CALL(name)		VIDEO_RESET_NAME(name)(machine)

#define PALETTE_INIT_NAME(name)		palette_init_##name
#define PALETTE_INIT(name)			void PALETTE_INIT_NAME(name)(running_machine &machine, const UINT8 *color_prom)
#define PALETTE_INIT_CALL(name)		PALETTE_INIT_NAME(name)(machine, color_prom)



// NULL versions
#define machine_start_0 			NULL
#define machine_reset_0 			NULL
#define sound_start_0				NULL
#define sound_reset_0				NULL
#define video_start_0				NULL
#define video_reset_0				NULL
#define palette_init_0				NULL



// global allocation helpers
#define auto_alloc(m, t)				pool_alloc(static_cast<running_machine &>(m).respool(), t)
#define auto_alloc_clear(m, t)			pool_alloc_clear(static_cast<running_machine &>(m).respool(), t)
#define auto_alloc_array(m, t, c)		pool_alloc_array(static_cast<running_machine &>(m).respool(), t, c)
#define auto_alloc_array_clear(m, t, c)	pool_alloc_array_clear(static_cast<running_machine &>(m).respool(), t, c)
#define auto_free(m, v)					pool_free(static_cast<running_machine &>(m).respool(), v)

#define auto_bitmap_alloc(m, w, h, f)	auto_alloc(m, bitmap_t(w, h, f))
#define auto_strdup(m, s)				strcpy(auto_alloc_array(m, char, strlen(s) + 1), s)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// forward declarations
class gfx_element;
class colortable_t;
class cheat_manager;
class render_manager;
class sound_manager;
class video_manager;
class debug_view_manager;
class osd_interface;

typedef struct _memory_private memory_private;
typedef struct _palette_private palette_private;
typedef struct _tilemap_private tilemap_private;
typedef struct _romload_private romload_private;
typedef struct _input_private input_private;
typedef struct _input_port_private input_port_private;
typedef struct _ui_input_private ui_input_private;
typedef struct _debugcpu_private debugcpu_private;
typedef struct _generic_machine_private generic_machine_private;
typedef struct _generic_video_private generic_video_private;
typedef struct _generic_audio_private generic_audio_private;


// legacy callback functions
typedef void   (*legacy_callback_func)(running_machine &machine);
typedef void   (*palette_init_func)(running_machine &machine, const UINT8 *color_prom);



// ======================> memory_region

// memory region object; should eventually be renamed memory_region
class memory_region
{
	DISABLE_COPYING(memory_region);

	friend class running_machine;
	friend class simple_list<memory_region>;
	friend resource_pool_object<memory_region>::~resource_pool_object();

	// construction/destruction
	memory_region(running_machine &machine, const char *name, UINT32 length, UINT8 width, endianness_t endian);
	~memory_region();

public:
	// getters
	memory_region *next() const { return m_next; }
	UINT8 *base() const { return (this != NULL) ? m_base.u8 : NULL; }
	UINT8 *end() const { return (this != NULL) ? m_base.u8 + m_length : NULL; }
	UINT32 bytes() const { return (this != NULL) ? m_length : 0; }
	const char *name() const { return m_name; }

	// flag expansion
	endianness_t endianness() const { return m_endianness; }
	UINT8 width() const { return m_width; }

	// data access
	UINT8 &u8(offs_t offset = 0) const { return m_base.u8[offset]; }
	UINT16 &u16(offs_t offset = 0) const { return m_base.u16[offset]; }
	UINT32 &u32(offs_t offset = 0) const { return m_base.u32[offset]; }
	UINT64 &u64(offs_t offset = 0) const { return m_base.u64[offset]; }

	// allow passing a region for any common pointer
	operator void *() const { return (this != NULL) ? m_base.v : NULL; }
	operator INT8 *() const { return (this != NULL) ? m_base.i8 : NULL; }
	operator UINT8 *() const { return (this != NULL) ? m_base.u8 : NULL; }
	operator INT16 *() const { return (this != NULL) ? m_base.i16 : NULL; }
	operator UINT16 *() const { return (this != NULL) ? m_base.u16 : NULL; }
	operator INT32 *() const { return (this != NULL) ? m_base.i32 : NULL; }
	operator UINT32 *() const { return (this != NULL) ? m_base.u32 : NULL; }
	operator INT64 *() const { return (this != NULL) ? m_base.i64 : NULL; }
	operator UINT64 *() const { return (this != NULL) ? m_base.u64 : NULL; }

private:
	// internal data
	running_machine &		m_machine;
	memory_region *			m_next;
	astring					m_name;
	generic_ptr				m_base;
	UINT32					m_length;
	UINT8					m_width;
	endianness_t			m_endianness;
};



// ======================> generic_pointers

// holds generic pointers that are commonly used
struct generic_pointers
{
	generic_ptr				spriteram;			// spriteram
	UINT32					spriteram_size;
	generic_ptr				spriteram2;			// secondary spriteram
	UINT32					spriteram2_size;
	generic_ptr				buffered_spriteram;	// buffered spriteram
	generic_ptr				buffered_spriteram2;// secondary buffered spriteram
	generic_ptr				paletteram;			// palette RAM
	generic_ptr				paletteram2;		// secondary palette RAM
	bitmap_t *				tmpbitmap;			// temporary bitmap
};



// ======================> system_time

// system time description, both local and UTC
class system_time
{
public:
	system_time();
	void set(time_t t);

	struct full_time
	{
		void set(struct tm &t);

		UINT8		second;		// seconds (0-59)
		UINT8		minute;		// minutes (0-59)
		UINT8		hour;		// hours (0-23)
		UINT8		mday;		// day of month (1-31)
		UINT8		month;		// month (0-11)
		INT32		year;		// year (1=1 AD)
		UINT8		weekday;	// day of week (0-6)
		UINT16		day;		// day of year (0-365)
		UINT8		is_dst;		// is this daylight savings?
	};

	INT64			time;		// number of seconds elapsed since midnight, January 1 1970 UTC
	full_time		local_time;	// local time
	full_time		utc_time;	// UTC coordinated time
};



// ======================> running_machine

// description of the currently-running machine
class running_machine : public bindable_object
{
	DISABLE_COPYING(running_machine);

	friend void debugger_init(running_machine &machine);
	friend class sound_manager;

	typedef void (*notify_callback)(running_machine &machine);
	typedef void (*logerror_callback)(running_machine &machine, const char *string);

	// must be at top of member variables
	resource_pool			m_respool;				// pool of resources for this machine

public:
	// construction/destruction
	running_machine(const machine_config &config, osd_interface &osd, bool exit_to_game_select = false);
	~running_machine();

	// getters
	const machine_config &config() const { return m_config; }
	const game_driver &system() const { return m_system; }
	osd_interface &osd() const { return m_osd; }
	resource_pool &respool() { return m_respool; }
	device_scheduler &scheduler() { return m_scheduler; }
	state_manager &state() { return m_state; }
	cheat_manager &cheat() const { assert(m_cheat != NULL); return *m_cheat; }
	render_manager &render() const { assert(m_render != NULL); return *m_render; }
	sound_manager &sound() const { assert(m_sound != NULL); return *m_sound; }
	video_manager &video() const { assert(m_video != NULL); return *m_video; }
	debug_view_manager &debug_view() const { assert(m_debug_view != NULL); return *m_debug_view; }
	driver_device *driver_data() const { return m_driver_device; }
	template<class T> T *driver_data() const { return downcast<T *>(m_driver_device); }
	machine_phase phase() const { return m_current_phase; }
    machine_init_phase init_phase() const { return m_current_init_phase; }
    int init_phase_percent_complete() const { return m_current_init_phase_pct_complete; }
	bool paused() const { return m_paused || (m_current_phase != MACHINE_PHASE_RUNNING); }
	bool exit_pending() const { return m_exit_pending; }
	bool new_driver_pending() const { return (m_new_driver_pending != NULL); }
	const char *new_driver_name() const { return m_new_driver_pending->name; }
	bool ui_active() const { return m_ui_active; }
	const char *basename() const { return m_basename; }
	int sample_rate() const { return m_sample_rate; }
	bool save_or_load_pending() const { return m_saveload_pending_file; }
	screen_device *first_screen() const { return primary_screen; }

	// additional helpers
	emu_options &options() const { return m_config.options(); }
	memory_region *first_region() const { return m_regionlist.first(); }
	attotime time() const { return m_scheduler.time(); }
	bool scheduled_event_pending() const { return m_exit_pending || m_hard_reset_pending; }

	// fetch items by name
	inline device_t *device(const char *tag);
	template<class T> inline T *device(const char *tag) { return downcast<T *>(device(tag)); }
	inline const input_port_config *port(const char *tag);
	inline const memory_region *region(const char *tag);

	// configuration helpers
	UINT32 total_colors() const { return m_config.m_total_colors; }

	// immediate operations
	int run(bool firstrun, bool benchmarking);
	void pause();
	void resume();
	void add_notifier(machine_notification event, notify_callback callback);
	void call_notifiers(machine_notification which);
	void add_logerror_callback(logerror_callback callback);
	void set_ui_active(bool active) { m_ui_active = active; }

	// scheduled operations
	void schedule_exit();
	void schedule_hard_reset();
	void schedule_soft_reset();
	void schedule_new_driver(const game_driver &driver);
	void schedule_save(const char *filename);
	void schedule_load(const char *filename);

	// date & time
	void base_datetime(system_time &systime);
	void current_datetime(system_time &systime);

	// regions
	memory_region *region_alloc(const char *name, UINT32 length, UINT8 width, endianness_t endian);
	void region_free(const char *name);

	// misc
	void CLIB_DECL logerror(const char *format, ...);
	void CLIB_DECL vlogerror(const char *format, va_list args);
	UINT32 rand();
	const char *describe_context();
    void announce_init_phase(machine_init_phase init_phase, int pct_complete);

	// internals
	device_list				m_devicelist;		// list of running devices
	ioport_list				m_portlist;			// points to a list of input port configurations

	// CPU information
	cpu_device *			firstcpu;			// first CPU (allows for quick iteration via typenext)

	// video-related information
	gfx_element *			gfx[MAX_GFX_ELEMENTS];// array of pointers to graphic sets (chars, sprites)
	screen_device *			primary_screen;		// the primary screen device, or NULL if screenless
	palette_t *				palette;			// global palette object

	// palette-related information
	const pen_t *			pens;				// remapped palette pen numbers
	colortable_t *			colortable;			// global colortable for remapping
	pen_t *					shadow_table;		// table for looking up a shadowed pen
	bitmap_t *				priority_bitmap;	// priority bitmap

	// debugger-related information
	UINT32					debug_flags;		// the current debug flags

	// generic pointers
	generic_pointers		generic;			// generic pointers

	// internal core information
	memory_private *		memory_data;		// internal data from memory.c
	palette_private *		palette_data;		// internal data from palette.c
	tilemap_private *		tilemap_data;		// internal data from tilemap.c
	romload_private *		romload_data;		// internal data from romload.c
	input_private *			input_data;			// internal data from input.c
	input_port_private *	input_port_data;	// internal data from inptport.c
	ui_input_private *		ui_input_data;		// internal data from uiinput.c
	debugcpu_private *		debugcpu_data;		// internal data from debugcpu.c
	generic_machine_private *generic_machine_data; // internal data from machine/generic.c
	generic_video_private *	generic_video_data;	// internal data from video/generic.c
	generic_audio_private *	generic_audio_data;	// internal data from audio/generic.c

private:
	// internal helpers
	void start();
	void set_saveload_filename(const char *filename);
	void fill_systime(system_time &systime, time_t t);
	void handle_saveload();
	void soft_reset(running_machine &machine, int param = 0);

	// internal callbacks
	static void logfile_callback(running_machine &machine, const char *buffer);

	// internal state
	const machine_config &	m_config;				// reference to the constructed machine_config
	const game_driver &		m_system;				// reference to the definition of the game machine
	osd_interface &			m_osd;					// reference to OSD system

	// embedded managers and objects
	tagged_list<memory_region> m_regionlist;		// list of memory regions
	state_manager			m_state;				// state manager
	device_scheduler		m_scheduler;			// scheduler object

	// managers
	cheat_manager *			m_cheat;				// internal data from cheat.c
	render_manager *		m_render;				// internal data from render.c
	sound_manager *			m_sound;				// internal data from sound.c
	video_manager *			m_video;				// internal data from video.c
	debug_view_manager *	m_debug_view;			// internal data from debugvw.c

	// driver state
	driver_device *			m_driver_device;		// pointer to the current driver device

	// system state
	machine_phase			m_current_phase;		// current execution phase
    machine_init_phase      m_current_init_phase;
    int                     m_current_init_phase_pct_complete;
	bool					m_paused;				// paused?
	bool					m_hard_reset_pending;	// is a hard reset pending?
	bool					m_exit_pending;			// is an exit pending?
	bool					m_exit_to_game_select;	// when we exit, go we go back to the game select?
	const game_driver *		m_new_driver_pending;	// pointer to the next pending driver
	emu_timer *				m_soft_reset_timer;		// timer used to schedule a soft reset

	// misc state
	UINT32					m_rand_seed;			// current random number seed
	bool					m_ui_active;			// ui active or not (useful for games / systems with keyboard inputs)
	time_t					m_base_time;			// real time at initial emulation time
	astring					m_basename;				// basename used for game-related paths
	astring					m_context;				// context string buffer
	int						m_sample_rate;			// the digital audio sample rate
	emu_file *				m_logfile;				// pointer to the active log file

	// load/save management
	enum saveload_schedule
	{
		SLS_NONE,
		SLS_SAVE,
		SLS_LOAD
	};
	saveload_schedule		m_saveload_schedule;
	attotime				m_saveload_schedule_time;
	astring					m_saveload_pending_file;
	const char *			m_saveload_searchpath;

	// notifier callbacks
	struct notifier_callback_item
	{
		// construction/destruction
		notifier_callback_item(notify_callback func);

		// getters
		notifier_callback_item *next() const { return m_next; }

		// state
		notifier_callback_item *	m_next;
		notify_callback				m_func;
	};
	simple_list<notifier_callback_item> m_notifier_list[MACHINE_NOTIFY_COUNT];

	// logerror callbacks
	class logerror_callback_item
	{
	public:
		// construction/destruction
		logerror_callback_item(logerror_callback func);

		// getters
		logerror_callback_item *next() const { return m_next; }

		// state
		logerror_callback_item *	m_next;
		logerror_callback			m_func;
	};
	simple_list<logerror_callback_item> m_logerror_list;
};



// ======================> driver_device_config_base

// a base class with common functionality for the (mostly stub) driver_device_configs
class driver_device_config_base : public device_config
{
	friend class driver_device;

protected:
	// construction/destruction
	driver_device_config_base(const machine_config &mconfig, device_type type, const char *tag, const device_config *owner);

public:
	// indexes into our generic callbacks
	enum callback_type
	{
		CB_MACHINE_START,
		CB_MACHINE_RESET,
		CB_SOUND_START,
		CB_SOUND_RESET,
		CB_VIDEO_START,
		CB_VIDEO_RESET,
		CB_COUNT
	};

	// inline configuration helpers
	static void static_set_game(device_config *device, const game_driver *game);
	static void static_set_callback(device_config *device, callback_type type, legacy_callback_func callback);
	static void static_set_palette_init(device_config *device, palette_init_func callback);

protected:
	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

	// internal state
	const game_driver *		m_system;						// pointer to the game driver

	legacy_callback_func	m_callbacks[CB_COUNT];		// generic legacy callbacks
	palette_init_func		m_palette_init;				// one-time palette init callback
};



// ======================> driver_device_config

// this provides a minimal config class for driver devices, which don't
// explicitly declare their own
template<class _DeviceClass>
class driver_device_config : public driver_device_config_base
{
	// construction/destruction
	driver_device_config(const machine_config &mconfig, const char *tag, const device_config *owner)
		: driver_device_config_base(mconfig, static_alloc_device_config, tag, owner) { }

public:
	// allocators
	static device_config *static_alloc_device_config(const machine_config &mconfig, const char *tag, const device_config *owner, UINT32 clock)
	{
		return global_alloc(driver_device_config(mconfig, tag, owner));
	}

	virtual device_t *alloc_device(running_machine &machine) const
	{
		// we clear here for historical reasons, as many existing driver states
		// assume everything is NULL before starting
		return auto_alloc_clear(machine, _DeviceClass(machine, *this));
	}
};



// ======================> driver_device

// base class for machine driver-specific devices
class driver_device : public device_t
{
public:
	// construction/destruction
	driver_device(running_machine &machine, const driver_device_config_base &config);
	virtual ~driver_device();

	// additional video helpers
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);
	virtual void screen_eof();

protected:
	// helpers called at startup
	virtual void driver_start();
	virtual void machine_start();
	virtual void sound_start();
	virtual void video_start();

	// helpers called at reset
	virtual void driver_reset();
	virtual void machine_reset();
	virtual void sound_reset();
	virtual void video_reset();

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// internal state
	const driver_device_config_base &m_config;
};



//**************************************************************************
//  INLINE FUNCTIONS
//**************************************************************************

inline device_t *running_machine::device(const char *tag)
{
	return m_devicelist.find(tag);
}

inline const input_port_config *running_machine::port(const char *tag)
{
	return m_portlist.find(tag);
}

inline const memory_region *running_machine::region(const char *tag)
{
	return m_regionlist.find(tag);
}


#endif	/* __MACHINE_H__ */
