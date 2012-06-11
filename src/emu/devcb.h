/***************************************************************************

    devcb.h

    Device callback interface helpers.

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

****************************************************************************

    These functions are used to adapt multiple read/write handler types
    to be used with device I/O. In general, a device is expected to
    declare its desired callback type, and these functions allow other
    callback types to be adapted appropriately.

    The desired callback types currently supported include:

        read_line_device_func:  (device)
        write_line_device_func: (device, data)
        read8_device_func:      (device, offset)
        write8_device_func:     (device, offset, data)
        read16_device_func:      (device, offset)
        write16_device_func:     (device, offset, data)

    The adapted callback types supported are:

        input port              (port)
        cpu input line          (cpu input line)
        read_line_device_func:  (device)
        write_line_device_func: (device, data)
        read8_device_func:      (device, offset)
        write8_device_func:     (device, offset, data)
        read8_space_func:       (space, offset)
        write8_space_func:      (space, offset, data)
        read16_device_func:     (device, offset)
        write16_device_func:    (device, offset, data)
        read16_space_func:      (space, offset)
        write16_space_func:     (space, offset, data)

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef __DEVCB_H__
#define __DEVCB_H__


//**************************************************************************
//  CONSTANTS
//**************************************************************************

// callback types
enum
{
	DEVCB_TYPE_NULL = 0,			// NULL callback
	DEVCB_TYPE_IOPORT,				// I/O port read/write
	DEVCB_TYPE_DEVICE,				// device read/write
	DEVCB_TYPE_LEGACY_SPACE,		// legacy address space read/write
	DEVCB_TYPE_INPUT_LINE,			// device input line write
	DEVCB_TYPE_CONSTANT				// constant value read
};



//**************************************************************************
//  MACROS
//**************************************************************************

// static template for a read_line stub function that calls through a given READ_LINE_MEMBER
template<class _Class, int (_Class::*_Function)()>
int devcb_line_stub(device_t *device)
{
	_Class *target = downcast<_Class *>(device);
	return (target->*_Function)();
}

// static template for a read8 stub function that calls through a given READ8_MEMBER
template<class _Class, UINT8 (_Class::*_Function)(address_space &, offs_t, UINT8)>
UINT8 devcb_stub(device_t *device, offs_t offset)
{
	_Class *target = downcast<_Class *>(device);
	return (target->*_Function)(*device->machine().memory().first_space(), offset, 0xff);
}

// static template for a read16 stub function that calls through a given READ16_MEMBER
template<class _Class, UINT16 (_Class::*_Function)(address_space &, offs_t, UINT16)>
UINT16 devcb_stub16(device_t *device, offs_t offset, UINT16 mask)
{
	_Class *target = downcast<_Class *>(device);
	return (target->*_Function)(*device->machine().memory().first_space(), offset, mask);
}

// static template for a write_line stub function that calls through a given WRITE_LINE_MEMBER
template<class _Class, void (_Class::*_Function)(int state)>
void devcb_line_stub(device_t *device, int state)
{
	_Class *target = downcast<_Class *>(device);
	(target->*_Function)(state);
}

// static template for a write8 stub function that calls through a given WRITE8_MEMBER
template<class _Class, void (_Class::*_Function)(address_space &, offs_t, UINT8, UINT8)>
void devcb_stub(device_t *device, offs_t offset, UINT8 data)
{
	_Class *target = downcast<_Class *>(device);
	(target->*_Function)(*device->machine().memory().first_space(), offset, data, 0xff);
}

// static template for a write16 stub function that calls through a given WRITE16_MEMBER
template<class _Class, void (_Class::*_Function)(address_space &, offs_t, UINT16, UINT16)>
void devcb_stub16(device_t *device, offs_t offset, UINT16 data, UINT16 mask)
{
	_Class *target = downcast<_Class *>(device);
	(target->*_Function)(*device->machine().memory().first_space(), offset, data, mask);
}

#define DEVCB_NULL								{ DEVCB_TYPE_NULL }

// standard line or read/write handlers with the calling device passed
#define DEVCB_LINE(func)						{ DEVCB_TYPE_DEVICE, 0, "", #func, func, NULL, NULL }
#define DEVCB_LINE_MEMBER(cls,memb)				{ DEVCB_TYPE_DEVICE, 0, "", #cls "::" #memb, &devcb_line_stub<cls, &cls::memb>, NULL, NULL }
#define DEVCB_HANDLER(func)						{ DEVCB_TYPE_DEVICE, 0, "", #func, NULL, func, NULL }
#define DEVCB_MEMBER(cls,memb)					{ DEVCB_TYPE_DEVICE, 0, "", #cls "::" #memb, NULL, &devcb_stub<cls, &cls::memb>, NULL }
#define DEVCB_MEMBER16(cls,memb)				{ DEVCB_TYPE_DEVICE, 0, "", #cls "::" #memb, NULL, &devcb_stub16<cls, &cls::memb>, NULL }

// line or read/write handlers for the driver device
#define DEVCB_DRIVER_LINE_MEMBER(cls,memb)		{ DEVCB_TYPE_DEVICE, 0, ":", #cls "::" #memb, &devcb_line_stub<cls, &cls::memb>, NULL, NULL }
#define DEVCB_DRIVER_MEMBER(cls,memb)			{ DEVCB_TYPE_DEVICE, 0, ":", #cls "::" #memb, NULL, &devcb_stub<cls, &cls::memb>, NULL }
#define DEVCB_DRIVER_MEMBER16(cls,memb)			{ DEVCB_TYPE_DEVICE, 0, ":", #cls "::" #memb, NULL, &devcb_stub16<cls, &cls::memb>, NULL }

// line or read/write handlers for another device
#define DEVCB_DEVICE_LINE(tag,func)				{ DEVCB_TYPE_DEVICE, 0, tag, #func, func, NULL, NULL }
#define DEVCB_DEVICE_LINE_MEMBER(tag,cls,memb)	{ DEVCB_TYPE_DEVICE, 0, tag, #cls "::" #memb, &devcb_line_stub<cls, &cls::memb>, NULL, NULL }
#define DEVCB_DEVICE_HANDLER(tag,func)			{ DEVCB_TYPE_DEVICE, 0, tag, #func, NULL, func, NULL }
#define DEVCB_DEVICE_MEMBER(tag,cls,memb)		{ DEVCB_TYPE_DEVICE, 0, tag, #cls "::" #memb, NULL, &devcb_stub<cls, &cls::memb>, NULL }
#define DEVCB_DEVICE_MEMBER16(tag,cls,memb)		{ DEVCB_TYPE_DEVICE, 0, tag, #cls "::" #memb, NULL, &devcb_stub16<cls, &cls::memb>, NULL }

// constant values
#define DEVCB_CONSTANT(value)					{ DEVCB_TYPE_CONSTANT, value, NULL, NULL, NULL, NULL }
#define DEVCB_LINE_GND							DEVCB_CONSTANT(0)
#define DEVCB_LINE_VCC							DEVCB_CONSTANT(1)

// read/write handlers for a given CPU's address space
#define DEVCB_MEMORY_HANDLER(cpu,space,func)	{ DEVCB_TYPE_LEGACY_SPACE, AS_##space, (cpu), #func, NULL, NULL, func }

// read handlers for an I/O port by tag
#define DEVCB_INPUT_PORT(tag)					{ DEVCB_TYPE_IOPORT, 0, (tag), NULL, NULL, NULL, NULL }

// write handlers for a CPU input line
#define DEVCB_CPU_INPUT_LINE(tag,line)			{ DEVCB_TYPE_INPUT_LINE, (line), (tag), NULL, NULL, NULL, NULL }


// macros for defining read_line/write_line functions
#define READ_LINE_DEVICE_HANDLER(name)		int  name(ATTR_UNUSED device_t *device)
#define WRITE_LINE_DEVICE_HANDLER(name) 	void name(ATTR_UNUSED device_t *device, ATTR_UNUSED int state)

#define DECLARE_READ_LINE_MEMBER(name)		int  name()
#define READ_LINE_MEMBER(name)				int  name()
#define DECLARE_WRITE_LINE_MEMBER(name) 	void name(ATTR_UNUSED int state)
#define WRITE_LINE_MEMBER(name)				void name(ATTR_UNUSED int state)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> devcb_resolved_objects

// resolving a devcb may produce one of the following object types
union devcb_resolved_objects
{
	ioport_port *	port;
	address_space *				space;
	device_t *					device;
	device_execute_interface *	execute;
	UINT32						constant;
};


// ======================> devcb_resolved_helpers

// resolving a devcb may produce one of the following helper functions/additional info
union devcb_resolved_read_helpers
{
	UINT8 *					null_indicator;
	read_line_device_func	read_line;
	read8_device_func		read8_device;
	read8_space_func		read8_space;
	read16_device_func		read16_device;
	read16_space_func		read16_space;
};

union devcb_resolved_write_helpers
{
	UINT8 *					null_indicator;
	write_line_device_func	write_line;
	write8_device_func		write8_device;
	write8_space_func		write8_space;
	write16_device_func		write16_device;
	write16_space_func		write16_space;
	int						input_line;
};


// ======================> devcb_read_line

// static structure used for device configuration when the desired callback type is a read_line_device_func
struct devcb_read_line
{
	UINT16					type;			// one of the special DEVCB_TYPE values
	UINT16					index;			// index related to the above types
	const char *			tag;			// tag of target, where appropriate
	const char *			name;			// name of the target function
	read_line_device_func	readline;		// read line function
	read8_device_func		readdevice;		// read device function
	read8_space_func		readspace;		// read space function
};


// ======================> devcb_resolved_read_line

// base delegate type for a read_line
typedef delegate<int ()> devcb_read_line_delegate;

// class which wraps resolving a devcb_read_line into a delegate
class devcb_resolved_read_line : public devcb_read_line_delegate
{
	DISABLE_COPYING(devcb_resolved_read_line);

public:
	// construction/destruction
	devcb_resolved_read_line();
	devcb_resolved_read_line(const devcb_read_line &desc, device_t &device) { resolve(desc, device); }

	// resolution
	void resolve(const devcb_read_line &desc, device_t &device);

	// override parent class' notion of NULL
	bool isnull() const { return m_helper.null_indicator == &s_null; }

private:
	// internal helpers
	int from_port();
	int from_read8();
	int from_constant();

	// internal state
	devcb_resolved_objects			m_object;
	devcb_resolved_read_helpers 	m_helper;
	static UINT8					s_null;
};


// ======================> devcb_write_line

// static structure used for device configuration when the desired callback type is a write_line_device_func
struct devcb_write_line
{
	UINT16					type;			// one of the special DEVCB_TYPE values
	UINT16					index;			// index related to the above types
	const char *			tag;			// tag of target, where appropriate
	const char *			name;			// name of the target function
	write_line_device_func	writeline;		// write line function
	write8_device_func		writedevice;	// write device function
	write8_space_func		writespace;		// write space function
};


// ======================> devcb_resolved_write_line

// base delegate type for a write_line
typedef delegate<void (int)> devcb_write_line_delegate;

// class which wraps resolving a devcb_write_line into a delegate
class devcb_resolved_write_line : public devcb_write_line_delegate
{
	DISABLE_COPYING(devcb_resolved_write_line);

public:
	// construction/destruction
	devcb_resolved_write_line();
	devcb_resolved_write_line(const devcb_write_line &desc, device_t &device) { resolve(desc, device); }

	// resolution
	void resolve(const devcb_write_line &desc, device_t &device);

	// override parent class' notion of NULL
	bool isnull() const { return m_helper.null_indicator == &s_null; }

private:
	// internal helpers
	void to_null(int state);
	void to_port(int state);
	void to_write8(int state);
	void to_input(int state);

	// internal state
	devcb_resolved_objects			m_object;
	devcb_resolved_write_helpers	m_helper;
	static UINT8					s_null;
};


// ======================> devcb_read8

// static structure used for device configuration when the desired callback type is a read8_device_func
struct devcb_read8
{
	UINT16					type;			// one of the special DEVCB_TYPE values
	UINT16					index;			// index related to the above types
	const char *			tag;			// tag of target, where appropriate
	const char *			name;			// name of the target function
	read_line_device_func	readline;		// read line function
	read8_device_func		readdevice;		// read device function
	read8_space_func		readspace;		// read space function
};


// ======================> devcb_resolved_read8

// base delegate type for a read8
typedef delegate<UINT8 (offs_t)> devcb_read8_delegate;

// class which wraps resolving a devcb_read8 into a delegate
class devcb_resolved_read8 : public devcb_read8_delegate
{
	DISABLE_COPYING(devcb_resolved_read8);

public:
	// construction/destruction
	devcb_resolved_read8();
	devcb_resolved_read8(const devcb_read8 &desc, device_t &device) { resolve(desc, device); }

	// resolution
	void resolve(const devcb_read8 &desc, device_t &device);

	// override parent class' notion of NULL
	bool isnull() const { return m_helper.null_indicator == &s_null; }

private:
	// internal helpers
	UINT8 from_port(offs_t offset);
	UINT8 from_readline(offs_t offset);
	UINT8 from_constant(offs_t offset);

	// internal state
	devcb_resolved_objects			m_object;
	devcb_resolved_read_helpers 	m_helper;
	static UINT8					s_null;
};


// ======================> devcb_write8

// static structure used for device configuration when the desired callback type is a write8_device_func
struct devcb_write8
{
	UINT16					type;			// one of the special DEVCB_TYPE values
	UINT16					index;			// index related to the above types
	const char *			tag;			// tag of target, where appropriate
	const char *			name;			// name of the target function
	write_line_device_func	writeline;		// write line function
	write8_device_func		writedevice;	// write device function
	write8_space_func		writespace;		// write space function
};


// ======================> devcb_resolved_write8

// base delegate type for a write8
typedef delegate<void (offs_t, UINT8)> devcb_write8_delegate;

// class which wraps resolving a devcb_write8 into a delegate
class devcb_resolved_write8 : public devcb_write8_delegate
{
	DISABLE_COPYING(devcb_resolved_write8);

public:
	// construction/destruction
	devcb_resolved_write8();
	devcb_resolved_write8(const devcb_write8 &desc, device_t &device) { resolve(desc, device); }

	// resolution
	void resolve(const devcb_write8 &desc, device_t &device);

	// override parent class' notion of NULL
	bool isnull() const { return m_helper.null_indicator == &s_null; }

private:
	// internal helpers
	void to_null(offs_t offset, UINT8 data);
	void to_port(offs_t offset, UINT8 data);
	void to_writeline(offs_t offset, UINT8 data);
	void to_input(offs_t offset, UINT8 data);

	// internal state
	devcb_resolved_objects			m_object;
	devcb_resolved_write_helpers	m_helper;
	static UINT8					s_null;
};


// ======================> devcb_read16

// static structure used for device configuration when the desired callback type is a read16_device_func
struct devcb_read16
{
	UINT16					type;			// one of the special DEVCB_TYPE values
	UINT16					index;			// index related to the above types
	const char *			tag;			// tag of target, where appropriate
	const char *			name;			// name of the target function
	read_line_device_func	readline;		// read line function
	read16_device_func		readdevice;		// read device function
	read16_space_func		readspace;		// read space function
};


// ======================> devcb_resolved_read16

// base delegate type for a write8
typedef delegate<UINT16 (offs_t, UINT16)> devcb_read16_delegate;

// class which wraps resolving a devcb_read16 into a delegate
class devcb_resolved_read16 : public devcb_read16_delegate
{
	DISABLE_COPYING(devcb_resolved_read16);

public:
	// construction/destruction
	devcb_resolved_read16();
	devcb_resolved_read16(const devcb_read16 &desc, device_t &device) { resolve(desc, device); }

	// resolution
	void resolve(const devcb_read16 &desc, device_t &device);

	// override parent class' notion of NULL
	bool isnull() const { return m_helper.null_indicator == &s_null; }

private:
	// internal helpers
	UINT16 from_port(offs_t offset, UINT16 mask);
	UINT16 from_readline(offs_t offset, UINT16 mask);
	UINT16 from_constant(offs_t offset, UINT16 mask);

	// internal state
	devcb_resolved_objects			m_object;
	devcb_resolved_read_helpers 	m_helper;
	static UINT8					s_null;
};


// ======================> devcb_write16

// static structure used for device configuration when the desired callback type is a write16_device_func
struct devcb_write16
{
	UINT16					type;			// one of the special DEVCB_TYPE values
	UINT16					index;			// index related to the above types
	const char *			tag;			// tag of target, where appropriate
	const char *			name;			// name of the target function
	write_line_device_func	writeline;		// write line function
	write16_device_func		writedevice;	// write device function
	write16_space_func		writespace;		// write space function
};


// ======================> devcb_resolved_write16

// base delegate type for a write8
typedef delegate<void (offs_t, UINT16, UINT16)> devcb_write16_delegate;

// class which wraps resolving a devcb_write16 into a delegate
class devcb_resolved_write16 : public devcb_write16_delegate
{
	DISABLE_COPYING(devcb_resolved_write16);

public:
	// construction/destruction
	devcb_resolved_write16();
	devcb_resolved_write16(const devcb_write16 &desc, device_t &device) { resolve(desc, device); }

	// resolution
	void resolve(const devcb_write16 &desc, device_t &device);

	// override parent class' notion of NULL
	bool isnull() const { return m_helper.null_indicator == &s_null; }

private:
	// internal helpers
	void to_null(offs_t offset, UINT16 data, UINT16 mask);
	void to_port(offs_t offset, UINT16 data, UINT16 mask);
	void to_writeline(offs_t offset, UINT16 data, UINT16 mask);
	void to_input(offs_t offset, UINT16 data, UINT16 mask);

	// internal state
	devcb_resolved_objects			m_object;
	devcb_resolved_write_helpers	m_helper;
	static UINT8					s_null;
};


#endif	// __DEVCB_H__
