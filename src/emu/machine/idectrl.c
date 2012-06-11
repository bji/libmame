/***************************************************************************

    Generic (PC-style) IDE controller implementation

***************************************************************************/

#include "emu.h"
#include "idectrl.h"
#include "debugger.h"
#include "imagedev/harddriv.h"

/***************************************************************************
    DEBUGGING
***************************************************************************/

#define VERBOSE						0
#define PRINTF_IDE_COMMANDS			0
#define PRINTF_IDE_PASSWORD			0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define LOGPRINT(x)	do { if (VERBOSE) logerror x; if (PRINTF_IDE_COMMANDS) mame_printf_debug x; } while (0)



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define IDE_DISK_SECTOR_SIZE			512

#define MINIMUM_COMMAND_TIME			(attotime::from_usec(10))

#define TIME_PER_SECTOR					(attotime::from_usec(100))
#define TIME_PER_ROTATION				(attotime::from_hz(5400/60))
#define TIME_SECURITY_ERROR				(attotime::from_msec(1000))

#define TIME_SEEK_MULTISECTOR			(attotime::from_msec(13))
#define TIME_NO_SEEK_MULTISECTOR		(attotime::from_nsec(16300))

#define IDE_STATUS_ERROR				0x01
#define IDE_STATUS_HIT_INDEX			0x02
#define IDE_STATUS_BUFFER_READY			0x08
#define IDE_STATUS_SEEK_COMPLETE		0x10
#define IDE_STATUS_DRIVE_READY			0x40
#define IDE_STATUS_BUSY					0x80

#define IDE_CONFIG_REGISTERS			0x10

#define BANK(b, v) (((v) << 4) | (b))

#define IDE_BANK0_DATA					BANK(0, 0)
#define IDE_BANK0_ERROR					BANK(0, 1)
#define IDE_BANK0_SECTOR_COUNT			BANK(0, 2)
#define IDE_BANK0_SECTOR_NUMBER			BANK(0, 3)
#define IDE_BANK0_CYLINDER_LSB			BANK(0, 4)
#define IDE_BANK0_CYLINDER_MSB			BANK(0, 5)
#define IDE_BANK0_HEAD_NUMBER			BANK(0, 6)
#define IDE_BANK0_STATUS_COMMAND		BANK(0, 7)

#define IDE_BANK1_STATUS_CONTROL		BANK(1, 6)

#define IDE_BANK2_CONFIG_UNK			BANK(2, 4)
#define IDE_BANK2_CONFIG_REGISTER		BANK(2, 8)
#define IDE_BANK2_CONFIG_DATA			BANK(2, 0xc)

#define IDE_COMMAND_READ_MULTIPLE		0x20
#define IDE_COMMAND_READ_MULTIPLE_ONCE	0x21
#define IDE_COMMAND_WRITE_MULTIPLE		0x30
#define IDE_COMMAND_DIAGNOSTIC			0x90
#define IDE_COMMAND_SET_CONFIG			0x91
#define IDE_COMMAND_READ_MULTIPLE_BLOCK	0xc4
#define IDE_COMMAND_WRITE_MULTIPLE_BLOCK 0xc5
#define IDE_COMMAND_SET_BLOCK_COUNT		0xc6
#define IDE_COMMAND_READ_DMA			0xc8
#define IDE_COMMAND_WRITE_DMA			0xca
#define IDE_COMMAND_GET_INFO			0xec
#define IDE_COMMAND_SET_FEATURES		0xef
#define IDE_COMMAND_SECURITY_UNLOCK		0xf2
#define IDE_COMMAND_UNKNOWN_F9			0xf9
#define IDE_COMMAND_VERIFY_MULTIPLE		0x40
#define IDE_COMMAND_VERIFY_NORETRY		0x41
#define IDE_COMMAND_ATAPI_IDENTIFY		0xa1
#define IDE_COMMAND_RECALIBRATE			0x10
#define IDE_COMMAND_SEEK				0x70
#define IDE_COMMAND_IDLE_IMMEDIATE		0xe1
#define IDE_COMMAND_IDLE				0xe3
#define IDE_COMMAND_TAITO_GNET_UNLOCK_1 0xfe
#define IDE_COMMAND_TAITO_GNET_UNLOCK_2 0xfc
#define IDE_COMMAND_TAITO_GNET_UNLOCK_3 0x0f

#define IDE_ERROR_NONE					0x00
#define IDE_ERROR_DEFAULT				0x01
#define IDE_ERROR_TRACK0_NOT_FOUND		0x02
#define IDE_ERROR_UNKNOWN_COMMAND		0x04
#define IDE_ERROR_BAD_LOCATION			0x10
#define IDE_ERROR_BAD_SECTOR			0x80

#define IDE_BUSMASTER_STATUS_ACTIVE		0x01
#define IDE_BUSMASTER_STATUS_ERROR		0x02
#define IDE_BUSMASTER_STATUS_IRQ		0x04



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
typedef struct _ide_device ide_device;
struct _ide_device
{
	UINT8			features[IDE_DISK_SECTOR_SIZE];

	UINT16			cur_cylinder;
	UINT8			cur_sector;
	UINT8			cur_head;
	UINT8			cur_head_reg;

	UINT32			cur_lba;

	UINT16			num_cylinders;
	UINT8			num_sectors;
	UINT8			num_heads;

	chd_file       *handle;
	hard_disk_file *disk;
	bool			is_image_device;
};


typedef struct _ide_state ide_state;
struct _ide_state
{
	device_t *device;

	UINT8			adapter_control;
	UINT8			status;
	UINT8			error;
	UINT8			command;
	UINT8			interrupt_pending;
	UINT8			precomp_offset;

	UINT8			buffer[IDE_DISK_SECTOR_SIZE];
	UINT16			buffer_offset;
	UINT16			sector_count;

	UINT16			block_count;
	UINT16			sectors_until_int;
	UINT8			verify_only;

	UINT8			dma_active;
	address_space *dma_space;
	UINT8			dma_address_xor;
	UINT8			dma_last_buffer;
	offs_t			dma_address;
	offs_t			dma_descriptor;
	UINT32			dma_bytes_left;

	UINT8			bus_master_command;
	UINT8			bus_master_status;
	UINT32			bus_master_descriptor;

	UINT8			config_unknown;
	UINT8			config_register[IDE_CONFIG_REGISTERS];
	UINT8			config_register_num;

	emu_timer *		last_status_timer;
	emu_timer *		reset_timer;

	UINT8			master_password_enable;
	UINT8			user_password_enable;
	const UINT8 *	master_password;
	const UINT8 *	user_password;

	UINT8			gnetreadlock;
	ide_hardware *	hardware;

	UINT8			cur_drive;
	ide_device		drive[2];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static TIMER_CALLBACK( reset_callback );

static void ide_build_features(ide_state *ide, int drive);

static void continue_read(ide_state *ide);
static void read_sector_done(ide_state *ide);
static TIMER_CALLBACK( read_sector_done_callback );
static void read_first_sector(ide_state *ide);
static void read_next_sector(ide_state *ide);

static UINT32 ide_controller_read(device_t *device, int bank, offs_t offset, int size);
static void ide_controller_write(device_t *device, int bank, offs_t offset, int size, UINT32 data);



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/*-------------------------------------------------
    get_safe_token - makes sure that the passed
    in device is, in fact, an IDE controller
-------------------------------------------------*/

INLINE ide_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == IDE_CONTROLLER);

	return (ide_state *)downcast<legacy_device_base *>(device)->token();
}


INLINE void signal_interrupt(ide_state *ide)
{
	const ide_config *config = (const ide_config *)downcast<const legacy_device_base *>(ide->device)->inline_config();

	LOG(("IDE interrupt assert\n"));

	/* signal an interrupt */
	if (config->interrupt != NULL)
		(*config->interrupt)(ide->device, ASSERT_LINE);
	ide->interrupt_pending = 1;
	ide->bus_master_status |= IDE_BUSMASTER_STATUS_IRQ;
}


INLINE void clear_interrupt(ide_state *ide)
{
	const ide_config *config = (const ide_config *)downcast<const legacy_device_base *>(ide->device)->inline_config();

	LOG(("IDE interrupt clear\n"));

	/* clear an interrupt */
	if (config->interrupt != NULL)
		(*config->interrupt)(ide->device, CLEAR_LINE);
	ide->interrupt_pending = 0;
}



/***************************************************************************
    DELAYED INTERRUPT HANDLING
***************************************************************************/

static TIMER_CALLBACK( delayed_interrupt )
{
	ide_state *ide = (ide_state *)ptr;
	ide->status &= ~IDE_STATUS_BUSY;
	signal_interrupt(ide);
}


static TIMER_CALLBACK( delayed_interrupt_buffer_ready )
{
	ide_state *ide = (ide_state *)ptr;
	ide->status &= ~IDE_STATUS_BUSY;
	ide->status |= IDE_STATUS_BUFFER_READY;
	signal_interrupt(ide);
}


INLINE void signal_delayed_interrupt(ide_state *ide, attotime time, int buffer_ready)
{
	/* clear buffer ready and set the busy flag */
	ide->status &= ~IDE_STATUS_BUFFER_READY;
	ide->status |= IDE_STATUS_BUSY;

	/* set a timer */
	if (buffer_ready)
		ide->device->machine().scheduler().timer_set(time, FUNC(delayed_interrupt_buffer_ready), 0, ide);
	else
		ide->device->machine().scheduler().timer_set(time, FUNC(delayed_interrupt), 0, ide);
}



/***************************************************************************
    INITIALIZATION AND RESET
***************************************************************************/

UINT8 *ide_get_features(device_t *device, int drive)
{
	ide_state *ide = get_safe_token(device);
	return ide->drive[drive].features;
}

void ide_set_gnet_readlock(device_t *device, const UINT8 onoff)
{
	ide_state *ide = get_safe_token(device);
	ide->gnetreadlock = onoff;
}

void ide_set_master_password(device_t *device, const UINT8 *password)
{
	ide_state *ide = get_safe_token(device);

	ide->master_password = password;
	ide->master_password_enable = (ide->master_password != NULL);
}


void ide_set_user_password(device_t *device, const UINT8 *password)
{
	ide_state *ide = get_safe_token(device);

	ide->user_password = password;
	ide->user_password_enable = (ide->user_password != NULL);
}


static TIMER_CALLBACK( reset_callback )
{
	reinterpret_cast<device_t *>(ptr)->reset();
}



/*************************************
 *
 *  Convert offset/mem_mask to offset
 *  and size
 *
 *************************************/

INLINE int convert_to_offset_and_size32(offs_t *offset, UINT32 mem_mask)
{
	int size = 4;

	/* determine which real offset */
	if (!ACCESSING_BITS_0_7)
	{
		(*offset)++, size = 3;
		if (!ACCESSING_BITS_8_15)
		{
			(*offset)++, size = 2;
			if (!ACCESSING_BITS_16_23)
				(*offset)++, size = 1;
		}
	}

	/* determine the real size */
	if (ACCESSING_BITS_24_31)
		return size;
	size--;
	if (ACCESSING_BITS_16_23)
		return size;
	size--;
	if (ACCESSING_BITS_8_15)
		return size;
	size--;
	return size;
}

INLINE int convert_to_offset_and_size16(offs_t *offset, UINT32 mem_mask)
{
	int size = 2;

	/* determine which real offset */
	if (!ACCESSING_BITS_0_7)
		(*offset)++, size = 1;

	if (ACCESSING_BITS_8_15)
		return size;
	size--;
	return size;
}



/*************************************
 *
 *  Compute the LBA address
 *
 *************************************/

INLINE UINT32 lba_address(ide_state *ide)
{
	/* LBA direct? */
	if (ide->drive[ide->cur_drive].cur_head_reg & 0x40)
		return ide->drive[ide->cur_drive].cur_sector + ide->drive[ide->cur_drive].cur_cylinder * 256 + ide->drive[ide->cur_drive].cur_head * 16777216;

	/* standard CHS */
	else
		return (ide->drive[ide->cur_drive].cur_cylinder * ide->drive[ide->cur_drive].num_heads + ide->drive[ide->cur_drive].cur_head) * ide->drive[ide->cur_drive].num_sectors + ide->drive[ide->cur_drive].cur_sector - 1;
}



/*************************************
 *
 *  Advance to the next sector
 *
 *************************************/

INLINE void next_sector(ide_state *ide)
{
	/* LBA direct? */
	if (ide->drive[ide->cur_drive].cur_head_reg & 0x40)
	{
		ide->drive[ide->cur_drive].cur_sector++;
		if (ide->drive[ide->cur_drive].cur_sector == 0)
		{
			ide->drive[ide->cur_drive].cur_cylinder++;
			if (ide->drive[ide->cur_drive].cur_cylinder == 0)
				ide->drive[ide->cur_drive].cur_head++;
		}
	}

	/* standard CHS */
	else
	{
		/* sectors are 1-based */
		ide->drive[ide->cur_drive].cur_sector++;
		if (ide->drive[ide->cur_drive].cur_sector > ide->drive[ide->cur_drive].num_sectors)
		{
			/* heads are 0 based */
			ide->drive[ide->cur_drive].cur_sector = 1;
			ide->drive[ide->cur_drive].cur_head++;
			if (ide->drive[ide->cur_drive].cur_head >= ide->drive[ide->cur_drive].num_heads)
			{
				ide->drive[ide->cur_drive].cur_head = 0;
				ide->drive[ide->cur_drive].cur_cylinder++;
			}
		}
	}

	ide->drive[ide->cur_drive].cur_lba = lba_address(ide);
}



/*************************************
 *
 *  Build a features page
 *
 *************************************/

static void swap_strncpy(UINT8 *dst, const char *src, int field_size_in_words)
{
	int i;

	assert(strlen(src) <= (field_size_in_words*2));

	for (i = 0; i < strlen(src); i++)
		dst[i ^ 1] = src[i];
	for ( ; i < field_size_in_words * 2; i++)
		dst[i ^ 1] = ' ';
}


static void ide_generate_features(ide_state *ide,int drive)
{
	int total_sectors = ide->drive[drive].num_cylinders * ide->drive[drive].num_heads * ide->drive[drive].num_sectors;
	int sectors_per_track = ide->drive[drive].num_heads * ide->drive[drive].num_sectors;

	/* basic geometry */
	ide->drive[drive].features[ 0*2+0] = 0x5a;						/*  0: configuration bits */
	ide->drive[drive].features[ 0*2+1] = 0x04;
	ide->drive[drive].features[ 1*2+0] = ide->drive[drive].num_cylinders & 0xff;	/*  1: logical cylinders */
	ide->drive[drive].features[ 1*2+1] = ide->drive[drive].num_cylinders >> 8;
	ide->drive[drive].features[ 2*2+0] = 0;							/*  2: reserved */
	ide->drive[drive].features[ 2*2+1] = 0;
	ide->drive[drive].features[ 3*2+0] = ide->drive[drive].num_heads & 0xff;		/*  3: logical heads */
	ide->drive[drive].features[ 3*2+1] = 0;/*ide->num_heads >> 8;*/
	ide->drive[drive].features[ 4*2+0] = 0;							/*  4: vendor specific (obsolete) */
	ide->drive[drive].features[ 4*2+1] = 0;
	ide->drive[drive].features[ 5*2+0] = 0;							/*  5: vendor specific (obsolete) */
	ide->drive[drive].features[ 5*2+1] = 0;
	ide->drive[drive].features[ 6*2+0] = ide->drive[drive].num_sectors & 0xff;	/*  6: logical sectors per logical track */
	ide->drive[drive].features[ 6*2+1] = 0;/*ide->num_sectors >> 8;*/
	ide->drive[drive].features[ 7*2+0] = 0;							/*  7: vendor-specific */
	ide->drive[drive].features[ 7*2+1] = 0;
	ide->drive[drive].features[ 8*2+0] = 0;							/*  8: vendor-specific */
	ide->drive[drive].features[ 8*2+1] = 0;
	ide->drive[drive].features[ 9*2+0] = 0;							/*  9: vendor-specific */
	ide->drive[drive].features[ 9*2+1] = 0;
	swap_strncpy(&ide->drive[drive].features[10*2+0],				/* 10-19: serial number */
			"00000000000000000000", 10);
	ide->drive[drive].features[20*2+0] = 0;							/* 20: vendor-specific */
	ide->drive[drive].features[20*2+1] = 0;
	ide->drive[drive].features[21*2+0] = 0;							/* 21: vendor-specific */
	ide->drive[drive].features[21*2+1] = 0;
	ide->drive[drive].features[22*2+0] = 4;							/* 22: # of vendor-specific bytes on read/write long commands */
	ide->drive[drive].features[22*2+1] = 0;
	swap_strncpy(&ide->drive[drive].features[23*2+0],				/* 23-26: firmware revision */
			"1.0", 4);
	swap_strncpy(&ide->drive[drive].features[27*2+0],				/* 27-46: model number */
			"MAME Compressed Hard Disk", 20);
	ide->drive[drive].features[47*2+0] = 0x01;						/* 47: read/write multiple support */
	ide->drive[drive].features[47*2+1] = 0x80;
	ide->drive[drive].features[48*2+0] = 0;							/* 48: reserved */
	ide->drive[drive].features[48*2+1] = 0;
	ide->drive[drive].features[49*2+0] = 0x03;						/* 49: capabilities */
	ide->drive[drive].features[49*2+1] = 0x0f;
	ide->drive[drive].features[50*2+0] = 0;							/* 50: reserved */
	ide->drive[drive].features[50*2+1] = 0;
	ide->drive[drive].features[51*2+0] = 2;							/* 51: PIO data transfer cycle timing mode */
	ide->drive[drive].features[51*2+1] = 0;
	ide->drive[drive].features[52*2+0] = 2;							/* 52: single word DMA transfer cycle timing mode */
	ide->drive[drive].features[52*2+1] = 0;
	ide->drive[drive].features[53*2+0] = 3;							/* 53: field validity */
	ide->drive[drive].features[53*2+1] = 0;
	ide->drive[drive].features[54*2+0] = ide->drive[drive].num_cylinders & 0xff;	/* 54: number of current logical cylinders */
	ide->drive[drive].features[54*2+1] = ide->drive[drive].num_cylinders >> 8;
	ide->drive[drive].features[55*2+0] = ide->drive[drive].num_heads & 0xff;		/* 55: number of current logical heads */
	ide->drive[drive].features[55*2+1] = 0;/*ide->num_heads >> 8;*/
	ide->drive[drive].features[56*2+0] = ide->drive[drive].num_sectors & 0xff;	/* 56: number of current logical sectors per track */
	ide->drive[drive].features[56*2+1] = 0;/*ide->num_sectors >> 8;*/
	ide->drive[drive].features[57*2+0] = sectors_per_track & 0xff;	/* 57-58: number of current logical sectors per track */
	ide->drive[drive].features[57*2+1] = sectors_per_track >> 8;
	ide->drive[drive].features[58*2+0] = sectors_per_track >> 16;
	ide->drive[drive].features[58*2+1] = sectors_per_track >> 24;
	ide->drive[drive].features[59*2+0] = 0;							/* 59: multiple sector timing */
	ide->drive[drive].features[59*2+1] = 0;
	ide->drive[drive].features[60*2+0] = total_sectors & 0xff;		/* 60-61: total user addressable sectors */
	ide->drive[drive].features[60*2+1] = total_sectors >> 8;
	ide->drive[drive].features[61*2+0] = total_sectors >> 16;
	ide->drive[drive].features[61*2+1] = total_sectors >> 24;
	ide->drive[drive].features[62*2+0] = 0x07;						/* 62: single word dma transfer */
	ide->drive[drive].features[62*2+1] = 0x00;
	ide->drive[drive].features[63*2+0] = 0x07;						/* 63: multiword DMA transfer */
	ide->drive[drive].features[63*2+1] = 0x04;
	ide->drive[drive].features[64*2+0] = 0x03;						/* 64: flow control PIO transfer modes supported */
	ide->drive[drive].features[64*2+1] = 0x00;
	ide->drive[drive].features[65*2+0] = 0x78;						/* 65: minimum multiword DMA transfer cycle time per word */
	ide->drive[drive].features[65*2+1] = 0x00;
	ide->drive[drive].features[66*2+0] = 0x78;						/* 66: mfr's recommended multiword DMA transfer cycle time */
	ide->drive[drive].features[66*2+1] = 0x00;
	ide->drive[drive].features[67*2+0] = 0x4d;						/* 67: minimum PIO transfer cycle time without flow control */
	ide->drive[drive].features[67*2+1] = 0x01;
	ide->drive[drive].features[68*2+0] = 0x78;						/* 68: minimum PIO transfer cycle time with IORDY */
	ide->drive[drive].features[68*2+1] = 0x00;
	ide->drive[drive].features[69*2+0] = 0x00;						/* 69-70: reserved */
	ide->drive[drive].features[69*2+1] = 0x00;
	ide->drive[drive].features[71*2+0] = 0x00;						/* 71: reserved for IDENTIFY PACKET command */
	ide->drive[drive].features[71*2+1] = 0x00;
	ide->drive[drive].features[72*2+0] = 0x00;						/* 72: reserved for IDENTIFY PACKET command */
	ide->drive[drive].features[72*2+1] = 0x00;
	ide->drive[drive].features[73*2+0] = 0x00;						/* 73: reserved for IDENTIFY PACKET command */
	ide->drive[drive].features[73*2+1] = 0x00;
	ide->drive[drive].features[74*2+0] = 0x00;						/* 74: reserved for IDENTIFY PACKET command */
	ide->drive[drive].features[74*2+1] = 0x00;
	ide->drive[drive].features[75*2+0] = 0x00;						/* 75: queue depth */
	ide->drive[drive].features[75*2+1] = 0x00;
	ide->drive[drive].features[76*2+0] = 0x00;						/* 76-79: reserved */
	ide->drive[drive].features[76*2+1] = 0x00;
	ide->drive[drive].features[80*2+0] = 0x00;						/* 80: major version number */
	ide->drive[drive].features[80*2+1] = 0x00;
	ide->drive[drive].features[81*2+0] = 0x00;						/* 81: minor version number */
	ide->drive[drive].features[81*2+1] = 0x00;
	ide->drive[drive].features[82*2+0] = 0x00;						/* 82: command set supported */
	ide->drive[drive].features[82*2+1] = 0x00;
	ide->drive[drive].features[83*2+0] = 0x00;						/* 83: command sets supported */
	ide->drive[drive].features[83*2+1] = 0x00;
	ide->drive[drive].features[84*2+0] = 0x00;						/* 84: command set/feature supported extension */
	ide->drive[drive].features[84*2+1] = 0x00;
	ide->drive[drive].features[85*2+0] = 0x00;						/* 85: command set/feature enabled */
	ide->drive[drive].features[85*2+1] = 0x00;
	ide->drive[drive].features[86*2+0] = 0x00;						/* 86: command set/feature enabled */
	ide->drive[drive].features[86*2+1] = 0x00;
	ide->drive[drive].features[87*2+0] = 0x00;						/* 87: command set/feature default */
	ide->drive[drive].features[87*2+1] = 0x00;
	ide->drive[drive].features[88*2+0] = 0x00;						/* 88: additional DMA modes */
	ide->drive[drive].features[88*2+1] = 0x00;
	ide->drive[drive].features[89*2+0] = 0x00;						/* 89: time required for security erase unit completion */
	ide->drive[drive].features[89*2+1] = 0x00;
	ide->drive[drive].features[90*2+0] = 0x00;						/* 90: time required for enhanced security erase unit completion */
	ide->drive[drive].features[90*2+1] = 0x00;
	ide->drive[drive].features[91*2+0] = 0x00;						/* 91: current advanced power management value */
	ide->drive[drive].features[91*2+1] = 0x00;
	ide->drive[drive].features[92*2+0] = 0x00;						/* 92: master password revision code */
	ide->drive[drive].features[92*2+1] = 0x00;
	ide->drive[drive].features[93*2+0] = 0x00;						/* 93: hardware reset result */
	ide->drive[drive].features[93*2+1] = 0x00;
	ide->drive[drive].features[94*2+0] = 0x00;						/* 94: acoustic management values */
	ide->drive[drive].features[94*2+1] = 0x00;
	ide->drive[drive].features[95*2+0] = 0x00;						/* 95-99: reserved */
	ide->drive[drive].features[95*2+1] = 0x00;
	ide->drive[drive].features[100*2+0] = total_sectors & 0xff;		/* 100-103: maximum 48-bit LBA */
	ide->drive[drive].features[100*2+1] = total_sectors >> 8;
	ide->drive[drive].features[101*2+0] = total_sectors >> 16;
	ide->drive[drive].features[101*2+1] = total_sectors >> 24;
	ide->drive[drive].features[102*2+0] = 0x00;
	ide->drive[drive].features[102*2+1] = 0x00;
	ide->drive[drive].features[103*2+0] = 0x00;
	ide->drive[drive].features[103*2+1] = 0x00;
	ide->drive[drive].features[104*2+0] = 0x00;						/* 104-126: reserved */
	ide->drive[drive].features[104*2+1] = 0x00;
	ide->drive[drive].features[127*2+0] = 0x00;						/* 127: removable media status notification */
	ide->drive[drive].features[127*2+1] = 0x00;
	ide->drive[drive].features[128*2+0] = 0x00;						/* 128: security status */
	ide->drive[drive].features[128*2+1] = 0x00;
	ide->drive[drive].features[129*2+0] = 0x00;						/* 129-159: vendor specific */
	ide->drive[drive].features[129*2+1] = 0x00;
	ide->drive[drive].features[160*2+0] = 0x00;						/* 160: CFA power mode 1 */
	ide->drive[drive].features[160*2+1] = 0x00;
	ide->drive[drive].features[161*2+0] = 0x00;						/* 161-175: reserved for CompactFlash */
	ide->drive[drive].features[161*2+1] = 0x00;
	ide->drive[drive].features[176*2+0] = 0x00;						/* 176-205: current media serial number */
	ide->drive[drive].features[176*2+1] = 0x00;
	ide->drive[drive].features[206*2+0] = 0x00;						/* 206-254: reserved */
	ide->drive[drive].features[206*2+1] = 0x00;
	ide->drive[drive].features[255*2+0] = 0x00;						/* 255: integrity word */
	ide->drive[drive].features[255*2+1] = 0x00;
}


static void ide_build_features(ide_state *ide, int drive)
{
	memset(ide->drive[drive].features, 0, IDE_DISK_SECTOR_SIZE);
	if (chd_get_metadata (ide->drive[drive].handle, HARD_DISK_IDENT_METADATA_TAG, 0, ide->drive[drive].features, IDE_DISK_SECTOR_SIZE, 0, 0, 0) != CHDERR_NONE)
		ide_generate_features (ide,drive);
}

/*************************************
 *
 *  security error handling
 *
 *************************************/

static TIMER_CALLBACK( security_error_done )
{
	ide_state *ide = (ide_state *)ptr;

	/* clear error state */
	ide->status &= ~IDE_STATUS_ERROR;
	ide->status |= IDE_STATUS_DRIVE_READY;
}

static void security_error(ide_state *ide)
{
	/* set error state */
	ide->status |= IDE_STATUS_ERROR;
	ide->status &= ~IDE_STATUS_DRIVE_READY;

	/* just set a timer and mark ourselves error */
	ide->device->machine().scheduler().timer_set(TIME_SECURITY_ERROR, FUNC(security_error_done), 0, ide);
}



/*************************************
 *
 *  Sector reading
 *
 *************************************/

static void continue_read(ide_state *ide)
{
	/* reset the totals */
	ide->buffer_offset = 0;

	/* clear the buffer ready and busy flag */
	ide->status &= ~IDE_STATUS_BUFFER_READY;
	ide->status &= ~IDE_STATUS_BUSY;

	if (ide->master_password_enable || ide->user_password_enable)
	{
		security_error(ide);

		ide->sector_count = 0;
		ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;
		ide->dma_active = 0;

		return;
	}

	/* if there is more data to read, keep going */
	if (ide->sector_count > 0)
		ide->sector_count--;
	if (ide->sector_count > 0)
		read_next_sector(ide);
	else
	{
		ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;
		ide->dma_active = 0;
	}
}


static void write_buffer_to_dma(ide_state *ide)
{
	int bytesleft = IDE_DISK_SECTOR_SIZE;
	UINT8 *data = ide->buffer;

//  LOG(("Writing sector to %08X\n", ide->dma_address));

	/* loop until we've consumed all bytes */
	while (bytesleft--)
	{
		/* if we're out of space, grab the next descriptor */
		if (ide->dma_bytes_left == 0)
		{
			/* if we're out of buffer space, that's bad */
			if (ide->dma_last_buffer)
			{
				LOG(("DMA Out of buffer space!\n"));
				return;
			}

			/* fetch the address */
			ide->dma_address = ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor);
			ide->dma_address |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 8;
			ide->dma_address |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 16;
			ide->dma_address |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 24;
			ide->dma_address &= 0xfffffffe;

			/* fetch the length */
			ide->dma_bytes_left = ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor);
			ide->dma_bytes_left |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 8;
			ide->dma_bytes_left |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 16;
			ide->dma_bytes_left |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 24;
			ide->dma_last_buffer = (ide->dma_bytes_left >> 31) & 1;
			ide->dma_bytes_left &= 0xfffe;
			if (ide->dma_bytes_left == 0)
				ide->dma_bytes_left = 0x10000;

//          LOG(("New DMA descriptor: address = %08X  bytes = %04X  last = %d\n", ide->dma_address, ide->dma_bytes_left, ide->dma_last_buffer));
		}

		/* write the next byte */
		ide->dma_space->write_byte(ide->dma_address++, *data++);
		ide->dma_bytes_left--;
	}
}


static void read_sector_done(ide_state *ide)
{
	int lba = lba_address(ide), count = 0;

	/* GNET readlock check */
	if (ide->gnetreadlock) {
		ide->status &= ~IDE_STATUS_ERROR;
		ide->status &= ~IDE_STATUS_BUSY;
		return;
	}
	/* now do the read */
	if (ide->drive[ide->cur_drive].disk)
		count = hard_disk_read(ide->drive[ide->cur_drive].disk, lba, ide->buffer);
	else if (ide->hardware != NULL) {
		count = ide->hardware->read_sector(ide->device, lba, ide->buffer);
	}

	/* by default, mark the buffer ready and the seek complete */
	if (!ide->verify_only)
		ide->status |= IDE_STATUS_BUFFER_READY;
	ide->status |= IDE_STATUS_SEEK_COMPLETE;

	/* and clear the busy and error flags */
	ide->status &= ~IDE_STATUS_ERROR;
	ide->status &= ~IDE_STATUS_BUSY;

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (ide->sector_count != 1)
			next_sector(ide);

		/* clear the error value */
		ide->error = IDE_ERROR_NONE;

		/* signal an interrupt */
		if (!ide->verify_only)
			ide->sectors_until_int--;
		if (ide->sectors_until_int == 0 || ide->sector_count == 1)
		{
			ide->sectors_until_int = ((ide->command == IDE_COMMAND_READ_MULTIPLE_BLOCK) ? ide->block_count : 1);
			signal_interrupt(ide);
		}

		/* handle DMA */
		if (ide->dma_active)
			write_buffer_to_dma(ide);

		/* if we're just verifying or if we DMA'ed the data, we can read the next sector */
		if (ide->verify_only || ide->dma_active)
			continue_read(ide);
	}

	/* if we got an error, we need to report it */
	else
	{
		/* set the error flag and the error */
		ide->status |= IDE_STATUS_ERROR;
		ide->error = IDE_ERROR_BAD_SECTOR;
		ide->bus_master_status |= IDE_BUSMASTER_STATUS_ERROR;
		ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;

		/* signal an interrupt */
		signal_interrupt(ide);
	}
}


static TIMER_CALLBACK( read_sector_done_callback )
{
	read_sector_done((ide_state *)ptr);
}


static void read_first_sector(ide_state *ide)
{
	/* mark ourselves busy */
	ide->status |= IDE_STATUS_BUSY;

	/* just set a timer */
	if (ide->command == IDE_COMMAND_READ_MULTIPLE_BLOCK)
	{
		int new_lba = lba_address(ide);
		attotime seek_time;

		if (new_lba == ide->drive[ide->cur_drive].cur_lba || new_lba == ide->drive[ide->cur_drive].cur_lba + 1)
			seek_time = TIME_NO_SEEK_MULTISECTOR;
		else
			seek_time = TIME_SEEK_MULTISECTOR;

		ide->drive[ide->cur_drive].cur_lba = new_lba;
		ide->device->machine().scheduler().timer_set(seek_time, FUNC(read_sector_done_callback), 0, ide);
	}
	else
		ide->device->machine().scheduler().timer_set(TIME_PER_SECTOR, FUNC(read_sector_done_callback), 0, ide);
}


static void read_next_sector(ide_state *ide)
{
	/* mark ourselves busy */
	ide->status |= IDE_STATUS_BUSY;

	if (ide->command == IDE_COMMAND_READ_MULTIPLE_BLOCK)
	{
		if (ide->sectors_until_int != 1)
			/* make ready now */
			read_sector_done(ide);
		else
			/* just set a timer */
			ide->device->machine().scheduler().timer_set(attotime::from_usec(1), FUNC(read_sector_done_callback), 0, ide);
	}
	else
		/* just set a timer */
		ide->device->machine().scheduler().timer_set(TIME_PER_SECTOR, FUNC(read_sector_done_callback), 0, ide);
}



/*************************************
 *
 *  Sector writing
 *
 *************************************/

static void write_sector_done(ide_state *ide);
static TIMER_CALLBACK( write_sector_done_callback );

static void continue_write(ide_state *ide)
{
	/* reset the totals */
	ide->buffer_offset = 0;

	/* clear the buffer ready flag */
	ide->status &= ~IDE_STATUS_BUFFER_READY;
	ide->status |= IDE_STATUS_BUSY;

	if (ide->command == IDE_COMMAND_WRITE_MULTIPLE_BLOCK)
	{
		if (ide->sectors_until_int != 1)
		{
			/* ready to write now */
			write_sector_done(ide);
		}
		else
		{
			/* set a timer to do the write */
			ide->device->machine().scheduler().timer_set(TIME_PER_SECTOR, FUNC(write_sector_done_callback), 0, ide);
		}
	}
	else
	{
		/* set a timer to do the write */
		ide->device->machine().scheduler().timer_set(TIME_PER_SECTOR, FUNC(write_sector_done_callback), 0, ide);
	}
}


static void read_buffer_from_dma(ide_state *ide)
{
	int bytesleft = IDE_DISK_SECTOR_SIZE;
	UINT8 *data = ide->buffer;

//  LOG(("Reading sector from %08X\n", ide->dma_address));

	/* loop until we've consumed all bytes */
	while (bytesleft--)
	{
		/* if we're out of space, grab the next descriptor */
		if (ide->dma_bytes_left == 0)
		{
			/* if we're out of buffer space, that's bad */
			if (ide->dma_last_buffer)
			{
				LOG(("DMA Out of buffer space!\n"));
				return;
			}

			/* fetch the address */
			ide->dma_address = ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor);
			ide->dma_address |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 8;
			ide->dma_address |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 16;
			ide->dma_address |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 24;
			ide->dma_address &= 0xfffffffe;

			/* fetch the length */
			ide->dma_bytes_left = ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor);
			ide->dma_bytes_left |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 8;
			ide->dma_bytes_left |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 16;
			ide->dma_bytes_left |= ide->dma_space->read_byte(ide->dma_descriptor++ ^ ide->dma_address_xor) << 24;
			ide->dma_last_buffer = (ide->dma_bytes_left >> 31) & 1;
			ide->dma_bytes_left &= 0xfffe;
			if (ide->dma_bytes_left == 0)
				ide->dma_bytes_left = 0x10000;

//          LOG(("New DMA descriptor: address = %08X  bytes = %04X  last = %d\n", ide->dma_address, ide->dma_bytes_left, ide->dma_last_buffer));
		}

		/* read the next byte */
		*data++ = ide->dma_space->read_byte(ide->dma_address++);
		ide->dma_bytes_left--;
	}
}


static void write_sector_done(ide_state *ide)
{
	int lba = lba_address(ide), count = 0;

	/* now do the write */
	if (ide->drive[ide->cur_drive].disk)
		count = hard_disk_write(ide->drive[ide->cur_drive].disk, lba, ide->buffer);
	else if (ide->hardware != NULL) {
		count = ide->hardware->write_sector(ide->device, lba, ide->buffer);
	}

	/* by default, mark the buffer ready and the seek complete */
	ide->status |= IDE_STATUS_BUFFER_READY;
	ide->status |= IDE_STATUS_SEEK_COMPLETE;

	/* and clear the busy adn error flags */
	ide->status &= ~IDE_STATUS_ERROR;
	ide->status &= ~IDE_STATUS_BUSY;

	/* if we succeeded, advance to the next sector and set the nice bits */
	if (count == 1)
	{
		/* advance the pointers, unless this is the last sector */
		/* Gauntlet: Dark Legacy checks to make sure we stop on the last sector */
		if (ide->sector_count != 1)
			next_sector(ide);

		/* clear the error value */
		ide->error = IDE_ERROR_NONE;

		/* signal an interrupt */
		if (--ide->sectors_until_int == 0 || ide->sector_count == 1)
		{
			ide->sectors_until_int = ((ide->command == IDE_COMMAND_WRITE_MULTIPLE_BLOCK) ? ide->block_count : 1);
			signal_interrupt(ide);
		}

		/* signal an interrupt if there's more data needed */
		if (ide->sector_count > 0)
			ide->sector_count--;
		if (ide->sector_count == 0)
			ide->status &= ~IDE_STATUS_BUFFER_READY;

		/* keep going for DMA */
		if (ide->dma_active && ide->sector_count != 0)
		{
			read_buffer_from_dma(ide);
			continue_write(ide);
		}
		else
			ide->dma_active = 0;
	}

	/* if we got an error, we need to report it */
	else
	{
		/* set the error flag and the error */
		ide->status |= IDE_STATUS_ERROR;
		ide->error = IDE_ERROR_BAD_SECTOR;
		ide->bus_master_status |= IDE_BUSMASTER_STATUS_ERROR;
		ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ACTIVE;

		/* signal an interrupt */
		signal_interrupt(ide);
	}
}


static TIMER_CALLBACK( write_sector_done_callback )
{
	write_sector_done((ide_state *)ptr);
}



/*************************************
 *
 *  Handle IDE commands
 *
 *************************************/

static void handle_command(ide_state *ide, UINT8 command)
{
	UINT8 key[5];

	/* implicitly clear interrupts here */
	clear_interrupt(ide);
	ide->command = command;
	switch (command)
	{
		case IDE_COMMAND_READ_MULTIPLE:
		case IDE_COMMAND_READ_MULTIPLE_ONCE:
			LOGPRINT(("IDE Read multiple: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->drive[ide->cur_drive].cur_cylinder, ide->drive[ide->cur_drive].cur_head, ide->drive[ide->cur_drive].cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;
			ide->verify_only = 0;

			/* start the read going */
			read_first_sector(ide);
			break;

		case IDE_COMMAND_READ_MULTIPLE_BLOCK:
			LOGPRINT(("IDE Read multiple block: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->drive[ide->cur_drive].cur_cylinder, ide->drive[ide->cur_drive].cur_head, ide->drive[ide->cur_drive].cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;
			ide->verify_only = 0;

			/* start the read going */
			read_first_sector(ide);
			break;

		case IDE_COMMAND_VERIFY_MULTIPLE:
		case IDE_COMMAND_VERIFY_NORETRY:
			LOGPRINT(("IDE Read verify multiple with/without retries: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->drive[ide->cur_drive].cur_cylinder, ide->drive[ide->cur_drive].cur_head, ide->drive[ide->cur_drive].cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;
			ide->verify_only = 1;

			/* start the read going */
			read_first_sector(ide);
			break;

		case IDE_COMMAND_READ_DMA:
			LOGPRINT(("IDE Read multiple DMA: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->drive[ide->cur_drive].cur_cylinder, ide->drive[ide->cur_drive].cur_head, ide->drive[ide->cur_drive].cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = ide->sector_count;
			ide->dma_active = 1;
			ide->verify_only = 0;

			/* start the read going */
			if (ide->bus_master_command & 1)
				read_first_sector(ide);
			break;

		case IDE_COMMAND_WRITE_MULTIPLE:
			LOGPRINT(("IDE Write multiple: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->drive[ide->cur_drive].cur_cylinder, ide->drive[ide->cur_drive].cur_head, ide->drive[ide->cur_drive].cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;

			/* mark the buffer ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			break;

		case IDE_COMMAND_WRITE_MULTIPLE_BLOCK:
			LOGPRINT(("IDE Write multiple block: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->drive[ide->cur_drive].cur_cylinder, ide->drive[ide->cur_drive].cur_head, ide->drive[ide->cur_drive].cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 1;
			ide->dma_active = 0;

			/* mark the buffer ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			break;

		case IDE_COMMAND_WRITE_DMA:
			LOGPRINT(("IDE Write multiple DMA: C=%d H=%d S=%d LBA=%d count=%d\n",
				ide->drive[ide->cur_drive].cur_cylinder, ide->drive[ide->cur_drive].cur_head, ide->drive[ide->cur_drive].cur_sector, lba_address(ide), ide->sector_count));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = ide->sector_count;
			ide->dma_active = 1;

			/* start the read going */
			if (ide->bus_master_command & 1)
			{
				read_buffer_from_dma(ide);
				continue_write(ide);
			}
			break;

		case IDE_COMMAND_SECURITY_UNLOCK:
			LOGPRINT(("IDE Security Unlock\n"));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 0;
			ide->dma_active = 0;

			/* mark the buffer ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_GET_INFO:
			LOGPRINT(("IDE Read features\n"));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sector_count = 1;

			/* build the features page */
			memcpy(ide->buffer, ide->drive[ide->cur_drive].features, sizeof(ide->buffer));

			/* indicate everything is ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			ide->status |= IDE_STATUS_SEEK_COMPLETE;
			ide->status |= IDE_STATUS_DRIVE_READY;

			/* and clear the busy adn error flags */
			ide->status &= ~IDE_STATUS_ERROR;
			ide->status &= ~IDE_STATUS_BUSY;

			/* clear the error too */
			ide->error = IDE_ERROR_NONE;

			/* signal an interrupt */
			signal_delayed_interrupt(ide, MINIMUM_COMMAND_TIME, 1);
			break;

		case IDE_COMMAND_DIAGNOSTIC:
			ide->error = IDE_ERROR_DEFAULT;

			/* signal an interrupt */
			signal_delayed_interrupt(ide, MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_RECALIBRATE:
			/* clear the error too */
			ide->error = IDE_ERROR_NONE;
			/* signal an interrupt */
			signal_delayed_interrupt(ide, MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_IDLE:
			/* clear the error too */
			ide->error = IDE_ERROR_NONE;

			/* for timeout disabled value is 0 */
			ide->sector_count = 0;
			/* signal an interrupt */
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_SET_CONFIG:
			LOGPRINT(("IDE Set configuration (%d heads, %d sectors)\n", ide->drive[ide->cur_drive].cur_head + 1, ide->sector_count));
			ide->status &= ~IDE_STATUS_ERROR;
			ide->error = IDE_ERROR_NONE;

			ide->drive[ide->cur_drive].num_sectors = ide->sector_count;
			ide->drive[ide->cur_drive].num_heads = ide->drive[ide->cur_drive].cur_head + 1;

			/* signal an interrupt */
			signal_delayed_interrupt(ide, MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_UNKNOWN_F9:
			/* only used by Killer Instinct AFAICT */
			LOGPRINT(("IDE unknown command (F9)\n"));

			/* signal an interrupt */
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_SET_FEATURES:
			LOGPRINT(("IDE Set features (%02X %02X %02X %02X %02X)\n", ide->precomp_offset, ide->sector_count & 0xff, ide->drive[ide->cur_drive].cur_sector, ide->drive[ide->cur_drive].cur_cylinder & 0xff, ide->drive[ide->cur_drive].cur_cylinder >> 8));

			/* signal an interrupt */
			signal_delayed_interrupt(ide, MINIMUM_COMMAND_TIME, 0);
			break;

		case IDE_COMMAND_SET_BLOCK_COUNT:
			LOGPRINT(("IDE Set block count (%02X)\n", ide->sector_count));

			ide->block_count = ide->sector_count;
			// judge dredd wants 'drive ready' on this command
			ide->status |= IDE_STATUS_DRIVE_READY;

			/* signal an interrupt */
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_TAITO_GNET_UNLOCK_1:
			LOGPRINT(("IDE GNET Unlock 1\n"));

			ide->sector_count = 1;
			ide->status |= IDE_STATUS_DRIVE_READY;
			ide->status &= ~IDE_STATUS_ERROR;
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_TAITO_GNET_UNLOCK_2:
			LOGPRINT(("IDE GNET Unlock 2\n"));

			/* reset the buffer */
			ide->buffer_offset = 0;
			ide->sectors_until_int = 0;
			ide->dma_active = 0;

			/* mark the buffer ready */
			ide->status |= IDE_STATUS_BUFFER_READY;
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_TAITO_GNET_UNLOCK_3:
			LOGPRINT(("IDE GNET Unlock 3\n"));

			/* key check */
			chd_get_metadata (ide->drive[ide->cur_drive].handle, HARD_DISK_KEY_METADATA_TAG, 0, key, 5, 0, 0, 0);
			if ((ide->precomp_offset == key[0]) && (ide->sector_count == key[1]) && (ide->drive[ide->cur_drive].cur_sector == key[2]) && (ide->drive[ide->cur_drive].cur_cylinder == (((UINT16)key[4]<<8)|key[3])))
			{
				ide->gnetreadlock= 0;
			}

			/* update flags */
			ide->status |= IDE_STATUS_DRIVE_READY;
			ide->status &= ~IDE_STATUS_ERROR;
			signal_interrupt(ide);
			break;

		case IDE_COMMAND_SEEK:
			/*
                cur_cylinder, cur_sector and cur_head
                are all already set in this case so no need
                so that implements actual seek
            */
			/* clear the error too */
			ide->error = IDE_ERROR_NONE;

			/* for timeout disabled value is 0 */
			ide->sector_count = 0;
			/* signal an interrupt */
			signal_interrupt(ide);
			break;


		default:
			LOGPRINT(("IDE unknown command (%02X)\n", command));
			ide->status |= IDE_STATUS_ERROR;
			ide->error = IDE_ERROR_UNKNOWN_COMMAND;
			signal_interrupt(ide);
			//debugger_break(ide->device->machine());
			break;
	}
}



/*************************************
 *
 *  IDE controller read
 *
 *************************************/

static UINT32 ide_controller_read(device_t *device, int bank, offs_t offset, int size)
{
	ide_state *ide = get_safe_token(device);
	UINT32 result = 0;

	/* logit */
//  if (BANK(bank, offset) != IDE_BANK0_DATA && BANK(bank, offset) != IDE_BANK0_STATUS_COMMAND && BANK(bank, offset) != IDE_BANK1_STATUS_CONTROL)
		LOG(("%s:IDE read at %d:%X, size=%d\n", device->machine().describe_context(), bank, offset, size));

	if (ide->drive[ide->cur_drive].disk) {
		ide->status |= IDE_STATUS_DRIVE_READY;
	} else {
		ide->status &= ~IDE_STATUS_DRIVE_READY;
	}

	switch (BANK(bank, offset))
	{
		/* unknown config register */
		case IDE_BANK2_CONFIG_UNK:
			return ide->config_unknown;

		/* active config register */
		case IDE_BANK2_CONFIG_REGISTER:
			return ide->config_register_num;

		/* data from active config register */
		case IDE_BANK2_CONFIG_DATA:
			if (ide->config_register_num < IDE_CONFIG_REGISTERS)
				return ide->config_register[ide->config_register_num];
			return 0;

		/* read data if there's data to be read */
		case IDE_BANK0_DATA:
			if (ide->status & IDE_STATUS_BUFFER_READY)
			{
				/* fetch the correct amount of data */
				result = ide->buffer[ide->buffer_offset++];
				if (size > 1)
					result |= ide->buffer[ide->buffer_offset++] << 8;
				if (size > 2)
				{
					result |= ide->buffer[ide->buffer_offset++] << 16;
					result |= ide->buffer[ide->buffer_offset++] << 24;
				}

				/* if we're at the end of the buffer, handle it */
				if (ide->buffer_offset >= IDE_DISK_SECTOR_SIZE)
				{
					LOG(("%s:IDE completed PIO read\n", device->machine().describe_context()));
					continue_read(ide);
					ide->error = IDE_ERROR_DEFAULT;
				}
			}
			break;

		/* return the current error */
		case IDE_BANK0_ERROR:
			return ide->error;

		/* return the current sector count */
		case IDE_BANK0_SECTOR_COUNT:
			return ide->sector_count;

		/* return the current sector */
		case IDE_BANK0_SECTOR_NUMBER:
			return ide->drive[ide->cur_drive].cur_sector;

		/* return the current cylinder LSB */
		case IDE_BANK0_CYLINDER_LSB:
			return ide->drive[ide->cur_drive].cur_cylinder & 0xff;

		/* return the current cylinder MSB */
		case IDE_BANK0_CYLINDER_MSB:
			return ide->drive[ide->cur_drive].cur_cylinder >> 8;

		/* return the current head */
		case IDE_BANK0_HEAD_NUMBER:
			return ide->drive[ide->cur_drive].cur_head_reg;

		/* return the current status and clear any pending interrupts */
		case IDE_BANK0_STATUS_COMMAND:
		/* return the current status but don't clear interrupts */
		case IDE_BANK1_STATUS_CONTROL:
			result = ide->status;
			if (ide->last_status_timer->elapsed() > TIME_PER_ROTATION)
			{
				result |= IDE_STATUS_HIT_INDEX;
				ide->last_status_timer->adjust(attotime::never);
			}

			/* clear interrutps only when reading the real status */
			if (BANK(bank, offset) == IDE_BANK0_STATUS_COMMAND)
			{
				if (ide->interrupt_pending)
					clear_interrupt(ide);
			}
			break;

		/* log anything else */
		default:
			logerror("%s:unknown IDE read at %03X, size=%d\n", device->machine().describe_context(), offset, size);
			break;
	}

	/* return the result */
	return result;
}



/*************************************
 *
 *  IDE controller write
 *
 *************************************/

static void ide_controller_write(device_t *device, int bank, offs_t offset, int size, UINT32 data)
{
	ide_state *ide = get_safe_token(device);

	/* logit */
	if (BANK(bank, offset) != IDE_BANK0_DATA)
		LOG(("%s:IDE write to %d:%X = %08X, size=%d\n", device->machine().describe_context(), bank, offset, data, size));
	//  fprintf(stderr, "ide write %03x %02x size=%d\n", offset, data, size);
	switch (BANK(bank, offset))
	{
		/* unknown config register */
		case IDE_BANK2_CONFIG_UNK:
			ide->config_unknown = data;
			break;

		/* active config register */
		case IDE_BANK2_CONFIG_REGISTER:
			ide->config_register_num = data;
			break;

		/* data from active config register */
		case IDE_BANK2_CONFIG_DATA:
			if (ide->config_register_num < IDE_CONFIG_REGISTERS)
				ide->config_register[ide->config_register_num] = data;
			break;

		/* write data */
		case IDE_BANK0_DATA:
			if (ide->status & IDE_STATUS_BUFFER_READY)
			{
				/* store the correct amount of data */
				ide->buffer[ide->buffer_offset++] = data;
				if (size > 1)
					ide->buffer[ide->buffer_offset++] = data >> 8;
				if (size > 2)
				{
					ide->buffer[ide->buffer_offset++] = data >> 16;
					ide->buffer[ide->buffer_offset++] = data >> 24;
				}

				/* if we're at the end of the buffer, handle it */
				if (ide->buffer_offset >= IDE_DISK_SECTOR_SIZE)
				{
					LOG(("%s:IDE completed PIO write\n", device->machine().describe_context()));
					if (ide->command == IDE_COMMAND_SECURITY_UNLOCK)
					{
						if (ide->user_password_enable && memcmp(ide->buffer, ide->user_password, 2 + 32) == 0)
						{
							LOGPRINT(("IDE Unlocked user password\n"));
							ide->user_password_enable = 0;
						}
						if (ide->master_password_enable && memcmp(ide->buffer, ide->master_password, 2 + 32) == 0)
						{
							LOGPRINT(("IDE Unlocked master password\n"));
							ide->master_password_enable = 0;
						}
						if (PRINTF_IDE_PASSWORD)
						{
							int i;

							for (i = 0; i < 34; i += 2)
							{
								if (i % 8 == 2)
									mame_printf_debug("\n");

								mame_printf_debug("0x%02x, 0x%02x, ", ide->buffer[i], ide->buffer[i + 1]);
								//mame_printf_debug("0x%02x%02x, ", ide->buffer[i], ide->buffer[i + 1]);
							}
							mame_printf_debug("\n");
						}

						/* clear the busy and error flags */
						ide->status &= ~IDE_STATUS_ERROR;
						ide->status &= ~IDE_STATUS_BUSY;
						ide->status &= ~IDE_STATUS_BUFFER_READY;

						if (ide->master_password_enable || ide->user_password_enable)
							security_error(ide);
						else
							ide->status |= IDE_STATUS_DRIVE_READY;
					}
					else if (ide->command == IDE_COMMAND_TAITO_GNET_UNLOCK_2)
					{
						UINT8 key[5] = { 0, 0, 0, 0, 0 };
						int i, bad = 0;
						chd_get_metadata (ide->drive[ide->cur_drive].handle, HARD_DISK_KEY_METADATA_TAG, 0, key, 5, 0, 0, 0);

						for (i=0; !bad && i<512; i++)
							bad = ((i < 2 || i >= 7) && ide->buffer[i]) || ((i >= 2 && i < 7) && ide->buffer[i] != key[i-2]);

						ide->status &= ~IDE_STATUS_BUSY;
						ide->status &= ~IDE_STATUS_BUFFER_READY;
						if (bad)
							ide->status |= IDE_STATUS_ERROR;
						else {
							ide->status &= ~IDE_STATUS_ERROR;
							ide->gnetreadlock= 0;
						}
					}
					else
						continue_write(ide);

				}
			}
			break;

		/* precompensation offset?? */
		case IDE_BANK0_ERROR:
			ide->precomp_offset = data;
			break;

		/* sector count */
		case IDE_BANK0_SECTOR_COUNT:
			ide->sector_count = data ? data : 256;
			break;

		/* current sector */
		case IDE_BANK0_SECTOR_NUMBER:
			ide->drive[ide->cur_drive].cur_sector = data;
			break;

		/* current cylinder LSB */
		case IDE_BANK0_CYLINDER_LSB:
			ide->drive[ide->cur_drive].cur_cylinder = (ide->drive[ide->cur_drive].cur_cylinder & 0xff00) | (data & 0xff);
			break;

		/* current cylinder MSB */
		case IDE_BANK0_CYLINDER_MSB:
			ide->drive[ide->cur_drive].cur_cylinder = (ide->drive[ide->cur_drive].cur_cylinder & 0x00ff) | ((data & 0xff) << 8);
			break;

		/* current head */
		case IDE_BANK0_HEAD_NUMBER:
			ide->cur_drive = (data & 0x10) >> 4;
			ide->drive[ide->cur_drive].cur_head = data & 0x0f;
			ide->drive[ide->cur_drive].cur_head_reg = data;
			// LBA mode = data & 0x40
			break;

		/* command */
		case IDE_BANK0_STATUS_COMMAND:
			handle_command(ide, data);
			break;

		/* adapter control */
		case IDE_BANK1_STATUS_CONTROL:
			ide->adapter_control = data;

			/* handle controller reset */
			//if (data == 0x04)
			if (data & 0x04)
			{
				ide->status |= IDE_STATUS_BUSY;
				ide->status &= ~IDE_STATUS_DRIVE_READY;
				ide->reset_timer->adjust(attotime::from_msec(5));
			}
			break;
	}
}



/*************************************
 *
 *  Bus master read
 *
 *************************************/

static UINT32 ide_bus_master_read(device_t *device, offs_t offset, int size)
{
	ide_state *ide = get_safe_token(device);

	LOG(("%s:ide_bus_master_read(%d, %d)\n", device->machine().describe_context(), offset, size));

	/* command register */
	if (offset == 0)
		return ide->bus_master_command | (ide->bus_master_status << 16);

	/* status register */
	if (offset == 2)
		return ide->bus_master_status;

	/* descriptor table register */
	if (offset == 4)
		return ide->bus_master_descriptor;

	return 0xffffffff;
}



/*************************************
 *
 *  Bus master write
 *
 *************************************/

static void ide_bus_master_write(device_t *device, offs_t offset, int size, UINT32 data)
{
	ide_state *ide = get_safe_token(device);

	LOG(("%s:ide_bus_master_write(%d, %d, %08X)\n", device->machine().describe_context(), offset, size, data));

	/* command register */
	if (offset == 0)
	{
		UINT8 old = ide->bus_master_command;
		UINT8 val = data & 0xff;

		/* save the read/write bit and the start/stop bit */
		ide->bus_master_command = (old & 0xf6) | (val & 0x09);
		ide->bus_master_status = (ide->bus_master_status & ~IDE_BUSMASTER_STATUS_ACTIVE) | (val & 0x01);

		/* handle starting a transfer */
		if (!(old & 1) && (val & 1))
		{
			/* reset all the DMA data */
			ide->dma_bytes_left = 0;
			ide->dma_last_buffer = 0;
			ide->dma_descriptor = ide->bus_master_descriptor;

			/* if we're going live, start the pending read/write */
			if (ide->dma_active)
			{
				if (ide->bus_master_command & 8)
					read_next_sector(ide);
				else
				{
					read_buffer_from_dma(ide);
					continue_write(ide);
				}
			}
		}
	}

	/* status register */
	if (offset <= 2 && offset + size > 2)
	{
		UINT8 old = ide->bus_master_status;
		UINT8 val = data >> (8 * (2 - offset));

		/* save the DMA capable bits */
		ide->bus_master_status = (old & 0x9f) | (val & 0x60);

		/* clear interrupt and error bits */
		if (val & IDE_BUSMASTER_STATUS_IRQ)
			ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_IRQ;
		if (val & IDE_BUSMASTER_STATUS_ERROR)
			ide->bus_master_status &= ~IDE_BUSMASTER_STATUS_ERROR;
	}

	/* descriptor table register */
	if (offset == 4)
		ide->bus_master_descriptor = data & 0xfffffffc;
}



/*************************************
 *
 *  IDE direct handlers (16-bit)
 *
 *************************************/

/*
    ide_bus_r()

    Read a 16-bit word from the IDE bus directly.

    select: 0->CS1Fx active, 1->CS3Fx active
    offset: register offset (state of DA2-DA0)
*/
int ide_bus_r(device_t *device, int select, int offset)
{
	return ide_controller_read(device, select ? 1 : 0, offset, select == 0 && offset == 0 ? 2 : 1);
}

/*
    ide_bus_w()

    Write a 16-bit word to the IDE bus directly.

    select: 0->CS1Fx active, 1->CS3Fx active
    offset: register offset (state of DA2-DA0)
    data: data written (state of D0-D15 or D0-D7)
*/
void ide_bus_w(device_t *device, int select, int offset, int data)
{
	if (select == 0 && offset == 0)
		ide_controller_write(device, 0, 0, 2, data);
	else
		ide_controller_write(device, select ? 1 : 0, offset, 1, data & 0xff);
}

UINT32 ide_controller_r(device_t *device, int reg, int size)
{
	if (reg >= 0x1f0 && reg < 0x1f8)
		return ide_controller_read(device, 0, reg & 7, size);
	if (reg >= 0x3f0 && reg < 0x3f8)
		return ide_controller_read(device, 1, reg & 7, size);
	if (reg >= 0x030 && reg < 0x040)
		return ide_controller_read(device, 2, reg & 0xf, size);
	return 0xffffffff;
}

void ide_controller_w(device_t *device, int reg, int size, UINT32 data)
{
	if (reg >= 0x1f0 && reg < 0x1f8)
		ide_controller_write(device, 0, reg & 7, size, data);
	if (reg >= 0x3f0 && reg < 0x3f8)
		ide_controller_write(device, 1, reg & 7, size, data);
	if (reg >= 0x030 && reg < 0x040)
		ide_controller_write(device, 2, reg & 0xf, size, data);
}


/*************************************
 *
 *  32-bit IDE handlers
 *
 *************************************/

READ32_DEVICE_HANDLER( ide_controller32_r )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	return ide_controller_r(device, offset, size) << ((offset & 3) * 8);
}


WRITE32_DEVICE_HANDLER( ide_controller32_w )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);
	data = data >> ((offset & 3) * 8);

	ide_controller_w(device, offset, size, data);
}


READ32_DEVICE_HANDLER( ide_controller32_pcmcia_r )
{
	int size;
	UINT32 res = 0xffffffff;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	if (offset >= 0x000 && offset < 0x008)
		res = ide_controller_read(device, 0, offset & 7, size);
	if (offset >= 0x008 && offset < 0x010)
		res = ide_controller_read(device, 1, offset & 7, size);

	return res << ((offset & 3) * 8);
}


WRITE32_DEVICE_HANDLER( ide_controller32_pcmcia_w )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);
	data = data >> ((offset & 3) * 8);

	if (offset >= 0x000 && offset < 0x008)
		ide_controller_write(device, 0, offset & 7, size, data);
	if (offset >= 0x008 && offset < 0x010)
		ide_controller_write(device, 1, offset & 7, size, data);
}

READ32_DEVICE_HANDLER( ide_bus_master32_r )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	return ide_bus_master_read(device, offset, size) << ((offset & 3) * 8);
}


WRITE32_DEVICE_HANDLER( ide_bus_master32_w )
{
	int size;

	offset *= 4;
	size = convert_to_offset_and_size32(&offset, mem_mask);

	ide_bus_master_write(device, offset, size, data >> ((offset & 3) * 8));
}



/*************************************
 *
 *  16-bit IDE handlers
 *
 *************************************/

READ16_DEVICE_HANDLER( ide_controller16_r )
{
	int size;

	offset *= 2;
	size = convert_to_offset_and_size16(&offset, mem_mask);

	return ide_controller_r(device, offset, size) << ((offset & 1) * 8);
}


WRITE16_DEVICE_HANDLER( ide_controller16_w )
{
	int size;

	offset *= 2;
	size = convert_to_offset_and_size16(&offset, mem_mask);

	ide_controller_w(device, offset, size, data >> ((offset & 1) * 8));
}



/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

static DEVICE_START( ide_controller )
{
	ide_state *ide = get_safe_token(device);
	const hard_disk_info *hdinfo;
	const ide_config *config;

	/* validate some basic stuff */
	assert(device != NULL);
	assert(device->static_config() == NULL);
	assert(downcast<const legacy_device_base *>(device)->inline_config() != NULL);

	/* store a pointer back to the device */
	ide->device = device;

	/* set MAME harddisk handle */
	config = (const ide_config *)downcast<const legacy_device_base *>(device)->inline_config();

	ide->drive[0].handle = get_disk_handle(device->machine(), (config->master != NULL) ? config->master : device->tag());
	ide->drive[0].disk = hard_disk_open(ide->drive[0].handle);
	ide->drive[0].is_image_device = false;

	if (config->slave) {
		ide->drive[1].handle = get_disk_handle(device->machine(), config->slave);
		ide->drive[1].disk = hard_disk_open(ide->drive[1].handle);
		ide->drive[1].is_image_device = false;
	}

	//assert_always(config->slave == NULL, "IDE controller does not yet support slave drives\n");

	/* find the bus master space */
	if (config->bmcpu != NULL)
	{
		device_t *bmtarget = device->machine().device(config->bmcpu);
		if (bmtarget == NULL)
			throw emu_fatalerror("IDE controller '%s' bus master target '%s' doesn't exist!", device->tag(), config->bmcpu);
		device_memory_interface *memory;
		if (!bmtarget->interface(memory))
			throw emu_fatalerror("IDE controller '%s' bus master target '%s' has no memory!", device->tag(), config->bmcpu);
		ide->dma_space = memory->space(config->bmspace);
		if (ide->dma_space == NULL)
			throw emu_fatalerror("IDE controller '%s' bus master target '%s' does not have specified space %d!", device->tag(), config->bmcpu, config->bmspace);
		ide->dma_address_xor = (ide->dma_space->endianness() == ENDIANNESS_LITTLE) ? 0 : 3;
	}

	/* get and copy the geometry */
	if (ide->drive[0].disk != NULL)
	{
		hdinfo = hard_disk_get_info(ide->drive[0].disk);
		if (hdinfo->sectorbytes == IDE_DISK_SECTOR_SIZE)
		{
			ide->drive[0].num_cylinders = hdinfo->cylinders;
			ide->drive[0].num_sectors = hdinfo->sectors;
			ide->drive[0].num_heads = hdinfo->heads;
			if (PRINTF_IDE_COMMANDS) mame_printf_debug("CHS: %d %d %d\n", ide->drive[0].num_cylinders, ide->drive[0].num_heads, ide->drive[0].num_sectors);
		}

		/* build the features page */
		ide_build_features(ide,0);
	}
	if (ide->drive[1].disk != NULL)
	{
		hdinfo = hard_disk_get_info(ide->drive[1].disk);
		if (hdinfo->sectorbytes == IDE_DISK_SECTOR_SIZE)
		{
			ide->drive[1].num_cylinders = hdinfo->cylinders;
			ide->drive[1].num_sectors = hdinfo->sectors;
			ide->drive[1].num_heads = hdinfo->heads;
			if (PRINTF_IDE_COMMANDS) mame_printf_debug("CHS: %d %d %d\n", ide->drive[1].num_cylinders, ide->drive[1].num_heads, ide->drive[1].num_sectors);
		}

		/* build the features page */
		ide_build_features(ide,1);
	}
	if (config->hardware != NULL) {
		ide->hardware = (ide_hardware *)config->hardware;
		ide->hardware->get_info(ide->device, ide->drive[0].features, ide->drive[0].num_cylinders, ide->drive[0].num_sectors, ide->drive[0].num_heads);
		ide_generate_features (ide,0);
	}

	/* create a timer for timing status */
	ide->last_status_timer = device->machine().scheduler().timer_alloc(FUNC_NULL);
	ide->reset_timer = device->machine().scheduler().timer_alloc(FUNC(reset_callback), (void *)device);

	/* register ide states */
	device->save_item(NAME(ide->adapter_control));
	device->save_item(NAME(ide->status));
	device->save_item(NAME(ide->error));
	device->save_item(NAME(ide->command));
	device->save_item(NAME(ide->interrupt_pending));
	device->save_item(NAME(ide->precomp_offset));

	device->save_item(NAME(ide->buffer));
	//device->save_item(NAME(ide->features));
	device->save_item(NAME(ide->buffer_offset));
	device->save_item(NAME(ide->sector_count));

	device->save_item(NAME(ide->block_count));
	device->save_item(NAME(ide->sectors_until_int));

	device->save_item(NAME(ide->dma_active));
	device->save_item(NAME(ide->dma_last_buffer));
	device->save_item(NAME(ide->dma_address));
	device->save_item(NAME(ide->dma_descriptor));
	device->save_item(NAME(ide->dma_bytes_left));

	device->save_item(NAME(ide->bus_master_command));
	device->save_item(NAME(ide->bus_master_status));
	device->save_item(NAME(ide->bus_master_descriptor));

	//device->save_item(NAME(ide->cur_cylinder));
	//device->save_item(NAME(ide->cur_sector));
	//device->save_item(NAME(ide->cur_head));
	//device->save_item(NAME(ide->cur_head_reg));

	//device->save_item(NAME(ide->cur_lba));

	//device->save_item(NAME(ide->num_cylinders));
	//device->save_item(NAME(ide->num_sectors));
	//device->save_item(NAME(ide->num_heads));

	device->save_item(NAME(ide->config_unknown));
	device->save_item(NAME(ide->config_register));
	device->save_item(NAME(ide->config_register_num));

	device->save_item(NAME(ide->master_password_enable));
	device->save_item(NAME(ide->user_password_enable));

	device->save_item(NAME(ide->gnetreadlock));
}


/*-------------------------------------------------
    device exit callback
-------------------------------------------------*/

static DEVICE_STOP( ide_controller )
{
	ide_state *ide = get_safe_token(device);
	if (!ide->drive[0].is_image_device) {
		/* close the hard disk */
		if (ide->drive[0].disk != NULL)
			hard_disk_close(ide->drive[0].disk);
	}
	if (!ide->drive[1].is_image_device) {
		/* close the hard disk */
		if (ide->drive[1].disk != NULL)
			hard_disk_close(ide->drive[1].disk);
	}
}


/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

static DEVICE_RESET( ide_controller )
{
	ide_state *ide = get_safe_token(device);
	const ide_config *config = (const ide_config *)downcast<const legacy_device_base *>(device)->inline_config();

	ide->cur_drive = 0;
	LOG(("IDE controller reset performed\n"));
	if (config->master) {
		astring hardtag_master;
		device->siblingtag(hardtag_master, config->master);
		if (!ide->drive[0].disk)
		{
			ide->drive[0].handle = device->machine().device<harddisk_image_device>(hardtag_master.cstr())->get_chd_file();	// should be config->master

			if (ide->drive[0].handle)
			{
				ide->drive[0].disk = device->machine().device<harddisk_image_device>(hardtag_master.cstr())->get_hard_disk_file();	// should be config->master
				ide->drive[0].is_image_device = true;

				if (ide->drive[0].disk != NULL)
				{
					const hard_disk_info *hdinfo;

					hdinfo = hard_disk_get_info(ide->drive[0].disk);
					if (hdinfo->sectorbytes == IDE_DISK_SECTOR_SIZE)
					{
						ide->drive[0].num_cylinders = hdinfo->cylinders;
						ide->drive[0].num_sectors = hdinfo->sectors;
						ide->drive[0].num_heads = hdinfo->heads;
						if (PRINTF_IDE_COMMANDS) printf("CHS: %d %d %d\n", ide->drive[0].num_cylinders, ide->drive[0].num_heads, ide->drive[0].num_sectors);
					}

					/* build the features page */
					ide_build_features(ide,0);
				}
			}
		}
	}
	if (config->slave) {
		astring hardtag_slave;
		device->siblingtag(hardtag_slave, config->slave);
		if (!ide->drive[1].disk)
		{
			ide->drive[1].handle = device->machine().device<harddisk_image_device>(hardtag_slave.cstr())->get_chd_file();	// should be config->master

			if (ide->drive[1].handle)
			{
				ide->drive[1].disk = device->machine().device<harddisk_image_device>(hardtag_slave.cstr())->get_hard_disk_file();	// should be config->master
				ide->drive[1].is_image_device = true;

				if (ide->drive[1].disk != NULL)
				{
					const hard_disk_info *hdinfo;

					hdinfo = hard_disk_get_info(ide->drive[1].disk);
					if (hdinfo->sectorbytes == IDE_DISK_SECTOR_SIZE)
					{
						ide->drive[1].num_cylinders = hdinfo->cylinders;
						ide->drive[1].num_sectors = hdinfo->sectors;
						ide->drive[1].num_heads = hdinfo->heads;
						if (PRINTF_IDE_COMMANDS) printf("CHS: %d %d %d\n", ide->drive[1].num_cylinders, ide->drive[1].num_heads, ide->drive[1].num_sectors);
					}

					/* build the features page */
					ide_build_features(ide,1);
				}
			}
		}
	}
	if (ide->hardware != NULL) {
		ide->hardware->get_info(ide->device, ide->drive[0].features, ide->drive[0].num_cylinders, ide->drive[0].num_sectors, ide->drive[0].num_heads);
		ide_generate_features (ide,0);
	}

	/* reset the drive state */
	ide->status = IDE_STATUS_DRIVE_READY | IDE_STATUS_SEEK_COMPLETE;
	ide->error = IDE_ERROR_DEFAULT;
	ide->buffer_offset = 0;
	ide->gnetreadlock = 0;
	ide->master_password_enable = (ide->master_password != NULL);
	ide->user_password_enable = (ide->user_password != NULL);
	clear_interrupt(ide);
}


/*-------------------------------------------------
    device get info callback
-------------------------------------------------*/

DEVICE_GET_INFO( ide_controller )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(ide_state);			break;
		case DEVINFO_INT_INLINE_CONFIG_BYTES:	info->i = sizeof(ide_config);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(ide_controller); break;
		case DEVINFO_FCT_STOP:					info->stop = DEVICE_STOP_NAME(ide_controller); break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(ide_controller);break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "IDE Controller");		break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Disk Controller");		break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");					break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright Nicola Salmoria and the MAME Team"); break;
	}
}


DEFINE_LEGACY_DEVICE(IDE_CONTROLLER, ide_controller);
