/*********************************************************************

    cassette.h

    Interface to the cassette image abstraction code

*********************************************************************/

#ifndef CASSETTE_H
#define CASSETTE_H

#include "image.h"
#include "formats/cassimg.h"


enum _cassette_state
{
	/* this part of the state is controlled by the UI */
	CASSETTE_STOPPED			= 0,
	CASSETTE_PLAY				= 1,
	CASSETTE_RECORD				= 2,

	/* this part of the state is controlled by drivers */
	CASSETTE_MOTOR_ENABLED		= 0,
	CASSETTE_MOTOR_DISABLED		= 4,
	CASSETTE_SPEAKER_ENABLED	= 0,
	CASSETTE_SPEAKER_MUTED		= 8,

	/* masks */
	CASSETTE_MASK_UISTATE		= 3,
	CASSETTE_MASK_MOTOR			= 4,
	CASSETTE_MASK_SPEAKER		= 8,
	CASSETTE_MASK_DRVSTATE		= 12
};

typedef enum _cassette_state cassette_state;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> cassette_interface

struct cassette_interface
{
	const struct CassetteFormat*	const *m_formats;
	const struct CassetteOptions	*m_create_opts;
	cassette_state					m_default_state;
	const char *					m_interface;
	device_image_display_info_func	m_device_displayinfo;
};

// ======================> cassette_image_device

class cassette_image_device :	public device_t,
								public cassette_interface,
								public device_image_interface
{
public:
	// construction/destruction
	cassette_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~cassette_image_device();

	// image-level overrides
	virtual bool call_load();
	virtual bool call_create(int format_type, option_resolution *format_options);
	virtual void call_unload();
	virtual void call_display();
	virtual void call_display_info() { if (m_device_displayinfo) m_device_displayinfo(*this); }
	virtual bool call_softlist_load(char *swlist, char *swname, rom_entry *start_entry) { return load_software(swlist, swname, start_entry); }

	virtual iodevice_t image_type() const { return IO_CASSETTE; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return m_interface; }
	virtual const char *file_extensions() const { return m_extension_list; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	// specific implementation
	cassette_state get_state() { return m_state; }
	void set_state(cassette_state state) { change_state(m_state, (cassette_state)(~0)); }
	void change_state(cassette_state state, cassette_state mask);

	double input();
	void output(double value);

	cassette_image *get_image() { return m_cassette; }
	double get_position();
	double get_length();
	void seek(double time, int origin);

protected:
	bool is_motor_on();
	void update();

	// device-level overrides
    virtual void device_config_complete();
	virtual void device_start();
private:
	cassette_image	*m_cassette;
	cassette_state	m_state;
	double			m_position;
	double			m_position_time;
	INT32			m_value;
	char			m_extension_list[256];
};

// device type definition
extern const device_type CASSETTE;

// device iterator
typedef device_type_iterator<&device_creator<cassette_image_device>, cassette_image_device> cassette_device_iterator;

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/
#define CASSETTE_TAG		"cassette"
#define CASSETTE2_TAG		"cassette2"

#define MCFG_CASSETTE_ADD(_tag, _config)	\
	MCFG_DEVICE_ADD(_tag, CASSETTE, 0)			\
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_CASSETTE_MODIFY(_tag, _config)	\
	MCFG_DEVICE_MODIFY(_tag)		\
	MCFG_DEVICE_CONFIG(_config)

extern const cassette_interface default_cassette_interface;

#endif /* CASSETTE_H */
