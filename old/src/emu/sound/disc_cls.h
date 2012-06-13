#pragma once

#ifndef __DISC_CLS_H__
#define __DISC_CLS_H__

/***********************************************************************
 *
 *  MAME - Discrete sound system emulation library
 *
 *  Written by Keith Wilkins (mame@esplexo.co.uk)
 *
 *  (c) K.Wilkins 2000
 *
 *  Coding started in November 2000
 *
 *  Additions/bugfix February 2003 - D.Renaud, F.Palazzolo, K.Wilkins
 *  Discrete parallel tasks 2009 - Couriersud
 *  Discrete classes 2010        - Couriersud
 *
 ***********************************************************************/

#define DISCRETE_CLASS_NAME(_name) discrete_ ## _name ## _node

#define DISCRETE_CLASS_INPUT(_name, _num)	inline double _name (void) { return *(m_input[_num]); }

#define DISCRETE_CLASS_CONSTRUCTOR(_name, _base)						\
	public:																\
		DISCRETE_CLASS_NAME(_name)()									\
		 : DISCRETE_CLASS_NAME(_base)() { }

#define DISCRETE_CLASS_DESTRUCTOR(_name)								\
	public:																\
		virtual ~ DISCRETE_CLASS_NAME(_name)(void) { }

#define  DISCRETE_CLASS_STEP_RESET(_name, _maxout, _priv)				\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface			\
{																		\
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)								\
	DISCRETE_CLASS_DESTRUCTOR(_name)									\
public:																	\
	void step(void);													\
	void reset(void);													\
	int max_output(void) { return _maxout; }					\
private:																\
	_priv																\
}

#define DISCRETE_CLASS_STEP(_name, _maxout, _priv)						\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface				\
{																		\
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)								\
	DISCRETE_CLASS_DESTRUCTOR(_name)									\
public:																	\
	void step(void);													\
	void reset(void)			{ this->step(); }						\
	int max_output(void) { return _maxout; }					\
private:																\
	_priv																\
}

#define  DISCRETE_CLASS_RESET(_name, _maxout)							\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node				\
{																		\
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)								\
	DISCRETE_CLASS_DESTRUCTOR(_name)									\
public:																	\
	void reset(void);													\
	int max_output(void) { return _maxout; }					\
}

#define  DISCRETE_CLASS(_name, _maxout, _priv)						\
class DISCRETE_CLASS_NAME(_name): public discrete_base_node, public discrete_step_interface				\
{																		\
	DISCRETE_CLASS_DESTRUCTOR(_name)									\
	DISCRETE_CLASS_CONSTRUCTOR(_name, base)								\
public:																	\
	void step(void);													\
	void reset(void);													\
	void start(void);													\
	void stop(void);													\
	int max_output(void) { return _maxout; }					\
private:																\
	_priv																\
}

class DISCRETE_CLASS_NAME(special): public discrete_base_node
{
	DISCRETE_CLASS_CONSTRUCTOR(special, base)
	DISCRETE_CLASS_DESTRUCTOR(special)
public:
	int max_output(void) { return 0; }
};

class DISCRETE_CLASS_NAME(unimplemented): public discrete_base_node
{
	DISCRETE_CLASS_CONSTRUCTOR(unimplemented, base)
	DISCRETE_CLASS_DESTRUCTOR(unimplemented)
public:
	int max_output(void) { return 0; }
};

/*************************************
 *
 *  disc_sys.c
 *
 *************************************/

class DISCRETE_CLASS_NAME(dso_output):	public discrete_base_node,
										public discrete_sound_output_interface,
										public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dso_output, base)
	DISCRETE_CLASS_DESTRUCTOR(dso_output)
public:
	void step(void) {
		/* Add gain to the output and put into the buffers */
		/* Clipping will be handled by the main sound system */
		double val = DISCRETE_INPUT(0) * DISCRETE_INPUT(1);
		*m_ptr++ = val;
	}
	int max_output(void) { return 0; }
	void set_output_ptr(stream_sample_t *ptr) { m_ptr = ptr; }
private:
	stream_sample_t		*m_ptr;
};

DISCRETE_CLASS(dso_csvlog, 0,
	FILE *m_csv_file;
	INT64 m_sample_num;
	char  m_name[32];
);

DISCRETE_CLASS(dso_wavlog, 0,
	wav_file *m_wavfile;
	char      m_name[32];
);

/*************************************
 *
 *  disc_inp.c
 *
 *************************************/

class DISCRETE_CLASS_NAME(dss_adjustment): public discrete_base_node, public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_adjustment, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_adjustment)
public:
	void step(void);
	void reset(void);
private:
	const input_port_config *m_port;
	INT32					m_lastpval;
	INT32					m_pmin;
	double					m_pscale;
	double					m_min;
	double					m_scale;
};

DISCRETE_CLASS_RESET(dss_constant, 1);

class DISCRETE_CLASS_NAME(dss_input_data): public discrete_base_node, public discrete_input_interface
{
	DISCRETE_CLASS_DESTRUCTOR(dss_input_data)
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_data, base)
public:
	void reset(void);
	void input_write(int sub_node, UINT8 data );
private:
	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
};

class DISCRETE_CLASS_NAME(dss_input_logic): public discrete_base_node, public discrete_input_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_logic, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_logic)
public:
	void reset(void);
	void input_write(int sub_node, UINT8 data );
private:
	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
};

class DISCRETE_CLASS_NAME(dss_input_not): public discrete_base_node, public discrete_input_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_not, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_not)
public:
	void reset(void);
	void input_write(int sub_node, UINT8 data );
private:
	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
};

class DISCRETE_CLASS_NAME(dss_input_pulse): public discrete_base_node, public discrete_input_interface, public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_pulse, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_pulse)
public:
	void step(void);
	void reset(void);
	void input_write(int sub_node, UINT8 data );
private:
	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
};

class DISCRETE_CLASS_NAME(dss_input_stream): public discrete_base_node, public discrete_input_interface, public discrete_step_interface
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_stream, base)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_stream)
public:
	void step(void);
	void reset(void);
	void start(void);
	void input_write(int sub_node, UINT8 data );
	virtual bool is_buffered(void) { return false; }

	/* This is called by discrete_sound_device */
	void stream_start(void);

//protected:
	UINT32				m_stream_in_number;
	stream_sample_t		*m_ptr;			/* current in ptr for stream */
private:
	static STREAM_UPDATE( static_stream_generate );
	void stream_generate(stream_sample_t **inputs, stream_sample_t **outputs, int samples);

	double		m_gain;				/* node gain */
	double		m_offset;			/* node offset */
	UINT8		m_data;				/* data written */
	UINT8				m_is_buffered;
	/* the buffer stream */
	sound_stream		*m_buffer_stream;
};

class DISCRETE_CLASS_NAME(dss_input_buffer): public DISCRETE_CLASS_NAME(dss_input_stream)
{
	DISCRETE_CLASS_CONSTRUCTOR(dss_input_buffer, dss_input_stream)
	DISCRETE_CLASS_DESTRUCTOR(dss_input_buffer)
public:
	bool is_buffered(void) { return true; }
};

#include "disc_wav.h"
#include "disc_mth.h"
#include "disc_flt.h"
#include "disc_dev.h"

#endif /* __DISCRETE_H__ */
