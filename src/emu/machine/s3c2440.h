/*******************************************************************************

    Samsung S3C2440

*******************************************************************************/

#ifndef __S3C2440_H__
#define __S3C2440_H__

/*******************************************************************************
    MACROS / CONSTANTS
*******************************************************************************/

#define S3C2440_TAG "s3c2440"

#define MCFG_S3C2440_ADD(_tag, _clock, _config) \
    MCFG_DEVICE_ADD(_tag, S3C2440, _clock) \
    MCFG_DEVICE_CONFIG(_config)

#define S3C2440_INTERFACE(name) \
	const s3c2440_interface(name) =

enum
{
	S3C2440_GPIO_PORT_A = 0,
	S3C2440_GPIO_PORT_B,
	S3C2440_GPIO_PORT_C,
	S3C2440_GPIO_PORT_D,
	S3C2440_GPIO_PORT_E,
	S3C2440_GPIO_PORT_F,
	S3C2440_GPIO_PORT_G,
	S3C2440_GPIO_PORT_H,
	S3C2440_GPIO_PORT_J
};

enum
{
	S3C2440_CORE_PIN_NCON = 0,
	S3C2440_CORE_PIN_OM0,
	S3C2440_CORE_PIN_OM1
};

DECLARE_LEGACY_DEVICE(S3C2440, s3c2440);

/*******************************************************************************
    TYPE DEFINITIONS
*******************************************************************************/

typedef UINT32 (*s3c24xx_gpio_port_r_func)( device_t *device, int port, UINT32 mask);
typedef void (*s3c24xx_gpio_port_w_func)( device_t *device, int port, UINT32 mask, UINT32 data);

typedef int (*s3c24xx_core_pin_r_func)( device_t *device, int pin);
typedef void (*s3c24xx_core_pin_w_func)( device_t *device, int pin, int data);

typedef struct _s3c2440_interface_core s3c2440_interface_core;
struct _s3c2440_interface_core
{
	s3c24xx_core_pin_r_func pin_r;
	s3c24xx_core_pin_w_func pin_w;
};

typedef struct _s3c2440_interface_gpio s3c2440_interface_gpio;
struct _s3c2440_interface_gpio
{
	s3c24xx_gpio_port_r_func port_r;
	s3c24xx_gpio_port_w_func port_w;
};

typedef struct _s3c2440_interface_i2c s3c2440_interface_i2c;
struct _s3c2440_interface_i2c
{
	write_line_device_func scl_w;
	read_line_device_func sda_r;
	write_line_device_func sda_w;
};

typedef struct _s3c2440_interface_adc s3c2440_interface_adc;
struct _s3c2440_interface_adc
{
	read32_device_func data_r;
};

typedef struct _s3c2440_interface_i2s s3c2440_interface_i2s;
struct _s3c2440_interface_i2s
{
	write16_device_func data_w;
};

typedef struct _s3c2440_interface_nand s3c2440_interface_nand;
struct _s3c2440_interface_nand
{
	write8_device_func command_w;
	write8_device_func address_w;
	read8_device_func data_r;
	write8_device_func data_w;
};

typedef struct _s3c2440_interface_lcd s3c2440_interface_lcd;
struct _s3c2440_interface_lcd
{
	int flags;
};

typedef struct _s3c2440_interface s3c2440_interface;
struct _s3c2440_interface
{
	s3c2440_interface_core core;
	s3c2440_interface_gpio gpio;
	s3c2440_interface_i2c i2c;
	s3c2440_interface_adc adc;
	s3c2440_interface_i2s i2s;
	s3c2440_interface_nand nand;
	s3c2440_interface_lcd lcd;
};

/*******************************************************************************
    PROTOTYPES
*******************************************************************************/

DEVICE_GET_INFO( s3c2440 );

VIDEO_START( s3c2440 );
SCREEN_UPDATE_RGB32( s3c2440 );

void s3c2440_uart_fifo_w( device_t *device, int uart, UINT8 data);
void s3c2440_touch_screen( device_t *device, int state);
void s3c2440_request_irq( device_t *device, UINT32 int_type);
void s3c2440_request_eint( device_t *device, UINT32 number);

WRITE_LINE_DEVICE_HANDLER( s3c2440_pin_frnb_w );

/*******************************************************************************
    MACROS & CONSTANTS
*******************************************************************************/

/* Interface */

#define S3C24XX_INTERFACE_LCD_REVERSE 1

/* Memory Controller */

#define S3C24XX_BASE_MEMCON 0x48000000

/* USB Host Controller */

#define S3C24XX_BASE_USBHOST 0x49000000

/* Interrupt Controller */

#define S3C24XX_BASE_INT 0x4A000000

#define S3C24XX_SRCPND    (0x00 / 4) // Interrupt Request Status
#define S3C24XX_INTMOD    (0x04 / 4) // Interrupt Mode Control
#define S3C24XX_INTMSK    (0x08 / 4) // Interrupt Mask Control
#define S3C24XX_PRIORITY  (0x0C / 4) // IRQ Priority Control
#define S3C24XX_INTPND    (0x10 / 4) // Interrupt Request Status
#define S3C24XX_INTOFFSET (0x14 / 4) // Interrupt Request Source Offset
#define S3C24XX_SUBSRCPND (0x18 / 4) // Sub Source Pending
#define S3C24XX_INTSUBMSK (0x1C / 4) // Interrupt Sub Mask

/* DMA */

#define S3C24XX_BASE_DMA_0 0x4B000000
#define S3C24XX_BASE_DMA_1 0x4B000040
#define S3C24XX_BASE_DMA_2 0x4B000080
#define S3C24XX_BASE_DMA_3 0x4B0000C0

#define S3C24XX_DISRC     (0x00 / 4) // DMA Initial Source
#define S3C24XX_DISRCC    (0x04 / 4) // DMA Initial Source Control
#define S3C24XX_DIDST     (0x08 / 4) // DMA Initial Destination
#define S3C24XX_DIDSTC    (0x0C / 4) // DMA Initial Destination Control
#define S3C24XX_DCON      (0x10 / 4) // DMA Control
#define S3C24XX_DSTAT     (0x14 / 4) // DMA Count
#define S3C24XX_DCSRC     (0x18 / 4) // DMA Current Source
#define S3C24XX_DCDST     (0x1C / 4) // DMA Current Destination
#define S3C24XX_DMASKTRIG (0x20 / 4) // DMA Mask Trigger

/* Clock & Power Management */

#define S3C24XX_BASE_CLKPOW 0x4C000000

#define S3C24XX_LOCKTIME (0x00 / 4) // PLL Lock Time Counter
#define S3C24XX_MPLLCON  (0x04 / 4) // MPLL Control
#define S3C24XX_UPLLCON  (0x08 / 4) // UPLL Control
#define S3C24XX_CLKCON   (0x0C / 4) // Clock Generator Control
#define S3C24XX_CLKSLOW  (0x10 / 4) // Slow Clock Control
#define S3C24XX_CLKDIVN  (0x14 / 4) // Clock Divider Control
#define S3C24XX_CAMDIVN  (0x18 / 4) // Camera Clock Divider Control

/* LCD Controller */

#define S3C24XX_BASE_LCD    0x4D000000
#define S3C24XX_BASE_LCDPAL 0x4D000400

#define S3C24XX_LCDCON1   (0x00 / 4) // LCD Control 1
#define S3C24XX_LCDCON2   (0x04 / 4) // LCD Control 2
#define S3C24XX_LCDCON3   (0x08 / 4) // LCD Control 3
#define S3C24XX_LCDCON4   (0x0C / 4) // LCD Control 4
#define S3C24XX_LCDCON5   (0x10 / 4) // LCD Control 5
#define S3C24XX_LCDSADDR1 (0x14 / 4) // STN/TFT: Frame Buffer Start Address 1
#define S3C24XX_LCDSADDR2 (0x18 / 4) // STN/TFT: Frame Buffer Start Address 2
#define S3C24XX_LCDSADDR3 (0x1C / 4) // STN/TFT: Virtual Screen Address Set
#define S3C24XX_REDLUT    (0x20 / 4) // STN: Red Lookup Table
#define S3C24XX_GREENLUT  (0x24 / 4) // STN: Green Lookup Table
#define S3C24XX_BLUELUT   (0x28 / 4) // STN: Blue Lookup Table
#define S3C24XX_DITHMODE  (0x4C / 4) // STN: Dithering Mode
#define S3C24XX_TPAL      (0x50 / 4) // TFT: Temporary Palette
#define S3C24XX_LCDINTPND (0x54 / 4) // LCD Interrupt Pending
#define S3C24XX_LCDSRCPND (0x58 / 4) // LCD Interrupt Source
#define S3C24XX_LCDINTMSK (0x5C / 4) // LCD Interrupt Mask
#define S3C24XX_TCONSEL   (0x60 / 4) // TCON (LPC3600/LCC3600) Control

/* NAND Flash */

#define S3C24XX_BASE_NAND 0x4E000000

#define S3C24XX_NFCONF   (0x00 / 4) // NAND Flash Configuration
#define S3C24XX_NFCONT   (0x04 / 4) // NAND Flash Control
#define S3C24XX_NFCMD    (0x08 / 4) // NAND Flash Command
#define S3C24XX_NFADDR   (0x0C / 4) // NAND Flash Address
#define S3C24XX_NFDATA   (0x10 / 4) // NAND Flash Data
#define S3C24XX_NFMECCD0 (0x14 / 4) // NAND Flash Main Area ECC0/1
#define S3C24XX_NFMECCD1 (0x18 / 4) // NAND Flash Main Area ECC2/3
#define S3C24XX_NFSECCD  (0x1C / 4) // NAND Flash Spare Area Ecc
#define S3C24XX_NFSTAT   (0x20 / 4) // NAND Flash Operation Status
#define S3C24XX_NFESTAT0 (0x24 / 4) // NAND Flash ECC Status For I/O[7:0]
#define S3C24XX_NFESTAT1 (0x28 / 4) // NAND Flash ECC Status For I/O[15:8]
#define S3C24XX_NFMECC0  (0x2C / 4) // NAND Flash Main Area ECC0 Status
#define S3C24XX_NFMECC1  (0x30 / 4) // NAND Flash Main Area ECC1 Status
#define S3C24XX_NFSECC   (0x34 / 4) // NAND Flash Spare Area ECC Status
#define S3C24XX_NFSBLK   (0x38 / 4) // NAND Flash Start Block Address
#define S3C24XX_NFEBLK   (0x3C / 4) // NAND Flash End Block Address

/* Camera Interface */

#define S3C24XX_BASE_CAM 0x4F000000

/* UART */

#define S3C24XX_BASE_UART_0 0x50000000
#define S3C24XX_BASE_UART_1 0x50004000
#define S3C24XX_BASE_UART_2 0x50008000

#define S3C24XX_ULCON   (0x00 / 4) // UART Line Control
#define S3C24XX_UCON    (0x04 / 4) // UART Control
#define S3C24XX_UFCON   (0x08 / 4) // UART FIFO Control
#define S3C24XX_UMCON   (0x0C / 4) // UART Modem Control
#define S3C24XX_UTRSTAT (0x10 / 4) // UART Tx/Rx Status
#define S3C24XX_UERSTAT (0x14 / 4) // UART Rx Error Status
#define S3C24XX_UFSTAT  (0x18 / 4) // UART FIFO Status
#define S3C24XX_UMSTAT  (0x1C / 4) // UART Modem Status
#define S3C24XX_UTXH    (0x20 / 4) // UART Transmission Hold
#define S3C24XX_URXH    (0x24 / 4) // UART Receive Buffer
#define S3C24XX_UBRDIV  (0x28 / 4) // UART Baud Rate Divisor

/* PWM Timer */

#define S3C24XX_BASE_PWM 0x51000000

#define S3C24XX_TCFG0  (0x00 / 4) // Timer Configuration
#define S3C24XX_TCFG1  (0x04 / 4) // Timer Configuration
#define S3C24XX_TCON   (0x08 / 4) // Timer Control
#define S3C24XX_TCNTB0 (0x0C / 4) // Timer Count Buffer 0
#define S3C24XX_TCMPB0 (0x10 / 4) // Timer Compare Buffer 0
#define S3C24XX_TCNTO0 (0x14 / 4) // Timer Count Observation 0
#define S3C24XX_TCNTB1 (0x18 / 4) // Timer Count Buffer 1
#define S3C24XX_TCMPB1 (0x1C / 4) // Timer Compare Buffer 1
#define S3C24XX_TCNTO1 (0x20 / 4) // Timer Count Observation 1
#define S3C24XX_TCNTB2 (0x24 / 4) // Timer Count Buffer 2
#define S3C24XX_TCMPB2 (0x28 / 4) // Timer Compare Buffer 2
#define S3C24XX_TCNTO2 (0x2C / 4) // Timer Count Observation 2
#define S3C24XX_TCNTB3 (0x30 / 4) // Timer Count Buffer 3
#define S3C24XX_TCMPB3 (0x34 / 4) // Timer Compare Buffer 3
#define S3C24XX_TCNTO3 (0x38 / 4) // Timer Count Observation 3
#define S3C24XX_TCNTB4 (0x3C / 4) // Timer Count Buffer 4
#define S3C24XX_TCNTO4 (0x40 / 4) // Timer Count Observation 4

/* USB Device */

#define S3C24XX_BASE_USBDEV 0x52000140

/* Watchdog Timer */

#define S3C24XX_BASE_WDT 0x53000000

#define S3C24XX_WTCON (0x00 / 4) // Watchdog Timer Mode
#define S3C24XX_WTDAT (0x04 / 4) // Watchdog Timer Data
#define S3C24XX_WTCNT (0x08 / 4) // Watchdog Timer Count

/* IIC */

#define S3C24XX_BASE_IIC 0x54000000

#define S3C24XX_IICCON  (0x00 / 4) // IIC Control
#define S3C24XX_IICSTAT (0x04 / 4) // IIC Status
#define S3C24XX_IICADD  (0x08 / 4) // IIC Address
#define S3C24XX_IICDS   (0x0C / 4) // IIC Data Shift
#define S3C24XX_IICLC   (0x10 / 4) // IIC Multi-Master Line Control

/* IIS */

#define S3C24XX_BASE_IIS 0x55000000

#define S3C24XX_IISCON  (0x00 / 4) // IIS Control
#define S3C24XX_IISMOD  (0x04 / 4) // IIS Mode
#define S3C24XX_IISPSR  (0x08 / 4) // IIS Prescaler
#define S3C24XX_IISFCON (0x0C / 4) // IIS FIFO Control
#define S3C24XX_IISFIFO (0x10 / 4) // IIS FIFO Entry

/* I/O Port */

#define S3C24XX_BASE_GPIO 0x56000000

#define S3C24XX_GPACON   (0x00 / 4) // Port A Control
#define S3C24XX_GPADAT   (0x04 / 4) // Port A Data
#define S3C24XX_GPBCON   (0x10 / 4) // Port B Control
#define S3C24XX_GPBDAT   (0x14 / 4) // Port B Data
#define S3C24XX_GPBUP    (0x18 / 4) // Pull-up Control B
#define S3C24XX_GPCCON   (0x20 / 4) // Port C Control
#define S3C24XX_GPCDAT   (0x24 / 4) // Port C Data
#define S3C24XX_GPCUP    (0x28 / 4) // Pull-up Control C
#define S3C24XX_GPDCON   (0x30 / 4) // Port D Control
#define S3C24XX_GPDDAT   (0x34 / 4) // Port D Data
#define S3C24XX_GPDUP    (0x38 / 4) // Pull-up Control D
#define S3C24XX_GPECON   (0x40 / 4) // Port E Control
#define S3C24XX_GPEDAT   (0x44 / 4) // Port E Data
#define S3C24XX_GPEUP    (0x48 / 4) // Pull-up Control E
#define S3C24XX_GPFCON   (0x50 / 4) // Port F Control
#define S3C24XX_GPFDAT   (0x54 / 4) // Port F Data
#define S3C24XX_GPFUP    (0x58 / 4) // Pull-up Control F
#define S3C24XX_GPGCON   (0x60 / 4) // Port G Control
#define S3C24XX_GPGDAT   (0x64 / 4) // Port G Data
#define S3C24XX_GPGUP    (0x68 / 4) // Pull-up Control G
#define S3C24XX_GPHCON   (0x70 / 4) // Port H Control
#define S3C24XX_GPHDAT   (0x74 / 4) // Port H Data
#define S3C24XX_GPHUP    (0x78 / 4) // Pull-up Control H
#define S3C24XX_MISCCR   (0x80 / 4) // Miscellaneous Control
#define S3C24XX_DCLKCON  (0x84 / 4) // DCLK0/1 Control
#define S3C24XX_EXTINT0  (0x88 / 4) // External Interrupt Control Register 0
#define S3C24XX_EXTINT1  (0x8C / 4) // External Interrupt Control Register 1
#define S3C24XX_EXTINT2  (0x90 / 4) // External Interrupt Control Register 2
#define S3C24XX_EINTFLT0 (0x94 / 4) // Reserved
#define S3C24XX_EINTFLT1 (0x98 / 4) // Reserved
#define S3C24XX_EINTFLT2 (0x9C / 4) // External Interrupt Filter Control Register 2
#define S3C24XX_EINTFLT3 (0xA0 / 4) // External Interrupt Filter Control Register 3
#define S3C24XX_EINTMASK (0xA4 / 4) // External Interrupt Mask
#define S3C24XX_EINTPEND (0xA8 / 4) // External Interrupt Pending
#define S3C24XX_GSTATUS0 (0xAC / 4) // External Pin Status
#define S3C24XX_GSTATUS1 (0xB0 / 4) // Chip ID
#define S3C24XX_GSTATUS2 (0xB4 / 4) // Reset Status
#define S3C24XX_GSTATUS3 (0xB8 / 4) // Inform Register
#define S3C24XX_GSTATUS4 (0xBC / 4) // Inform Register
#define S3C24XX_MSLCON   (0xCC / 4) // Memory Sleep Control Register
#define S3C24XX_GPJCON   (0xD0 / 4) // Port J Control
#define S3C24XX_GPJDAT   (0xD4 / 4) // Port J Data
#define S3C24XX_GPJUP    (0xD8 / 4) // Pull-up Control J

#define S3C24XX_GPADAT_MASK 0x01FFFFFF
#define S3C24XX_GPBDAT_MASK 0x000007FF
#define S3C24XX_GPCDAT_MASK 0x0000FFFF
#define S3C24XX_GPDDAT_MASK 0x0000FFFF
#define S3C24XX_GPEDAT_MASK 0x0000FFFF
#define S3C24XX_GPFDAT_MASK 0x000000FF
#define S3C24XX_GPGDAT_MASK 0x0000FFFF
#define S3C24XX_GPHDAT_MASK 0x000007FF
#define S3C24XX_GPJDAT_MASK 0x0000FFFF

/* RTC */

#define S3C24XX_BASE_RTC 0x57000040

#define S3C24XX_RTCCON  (0x00 / 4) // RTC Control
#define S3C24XX_TICNT   (0x04 / 4) // Tick Time count
#define S3C24XX_RTCALM  (0x10 / 4) // RTC Alarm Control
#define S3C24XX_ALMSEC  (0x14 / 4) // Alarm Second
#define S3C24XX_ALMMIN  (0x18 / 4) // Alarm Minute
#define S3C24XX_ALMHOUR (0x1C / 4) // Alarm Hour
#define S3C24XX_ALMDAY  (0x20 / 4) // Alarm Day
#define S3C24XX_ALMMON  (0x24 / 4) // Alarm Month
#define S3C24XX_ALMYEAR (0x28 / 4) // Alarm Year
#define S3C24XX_BCDSEC  (0x30 / 4) // BCD Second
#define S3C24XX_BCDMIN  (0x34 / 4) // BCD Minute
#define S3C24XX_BCDHOUR (0x38 / 4) // BCD Hour
#define S3C24XX_BCDDAY  (0x3C / 4) // BCD Day
#define S3C24XX_BCDDOW  (0x40 / 4) // BCD Day of Week
#define S3C24XX_BCDMON  (0x44 / 4) // BCD Month
#define S3C24XX_BCDYEAR (0x48 / 4) // BCD Year

/* A/D Converter */

#define S3C24XX_BASE_ADC 0x58000000

#define S3C24XX_ADCCON  (0x00 / 4) // ADC Control
#define S3C24XX_ADCTSC  (0x04 / 4) // ADC Touch Screen Control
#define S3C24XX_ADCDLY  (0x08 / 4) // ADC Start or Interval Delay
#define S3C24XX_ADCDAT0 (0x0C / 4) // ADC Conversion Data
#define S3C24XX_ADCDAT1 (0x10 / 4) // ADC Conversion Data
#define S3C24XX_ADCUPDN (0x14 / 4) // Stylus up or down interrupt status

/* SPI */

#define S3C24XX_BASE_SPI_0 0x59000000
#define S3C24XX_BASE_SPI_1 0x59000020

#define S3C24XX_SPCON  (0x00 / 4) // SPI Control
#define S3C24XX_SPSTA  (0x04 / 4) // SPI Status
#define S3C24XX_SPPIN  (0x08 / 4) // SPI Pin Control
#define S3C24XX_SPPRE  (0x0C / 4) // SPI Baud Rate Prescaler
#define S3C24XX_SPTDAT (0x10 / 4) // SPI Tx Data
#define S3C24XX_SPRDAT (0x14 / 4) // SPI Rx Data

/* SD Interface */

#define S3C24XX_BASE_SDI 0x5A000000

/* AC97 Interface */

#define S3C24XX_BASE_AC97 0x5B000000

/* ... */

#define S3C24XX_INT_ADC       31
#define S3C24XX_INT_RTC       30
#define S3C24XX_INT_SPI1      29
#define S3C24XX_INT_UART0     28
#define S3C24XX_INT_IIC       27
#define S3C24XX_INT_USBH      26
#define S3C24XX_INT_USBD      25
#define S3C24XX_INT_NFCON     24
#define S3C24XX_INT_UART1     23
#define S3C24XX_INT_SPI0      22
#define S3C24XX_INT_SDI       21
#define S3C24XX_INT_DMA3      20
#define S3C24XX_INT_DMA2      19
#define S3C24XX_INT_DMA1      18
#define S3C24XX_INT_DMA0      17
#define S3C24XX_INT_LCD       16
#define S3C24XX_INT_UART2     15
#define S3C24XX_INT_TIMER4    14
#define S3C24XX_INT_TIMER3    13
#define S3C24XX_INT_TIMER2    12
#define S3C24XX_INT_TIMER1    11
#define S3C24XX_INT_TIMER0    10
#define S3C24XX_INT_WDT_AC97   9
#define S3C24XX_INT_TICK       8
#define S3C24XX_INT_BATT_FLT   7
#define S3C24XX_INT_CAM        6
#define S3C24XX_INT_EINT8_23   5
#define S3C24XX_INT_EINT4_7    4
#define S3C24XX_INT_EINT3      3
#define S3C24XX_INT_EINT2      2
#define S3C24XX_INT_EINT1      1
#define S3C24XX_INT_EINT0      0

#define S3C24XX_SUBINT_AC97   14
#define S3C24XX_SUBINT_WDT    13
#define S3C24XX_SUBINT_CAM_P  12
#define S3C24XX_SUBINT_CAM_C  11
#define S3C24XX_SUBINT_ADC    10
#define S3C24XX_SUBINT_TC      9
#define S3C24XX_SUBINT_ERR2    8
#define S3C24XX_SUBINT_TXD2    7
#define S3C24XX_SUBINT_RXD2    6
#define S3C24XX_SUBINT_ERR1    5
#define S3C24XX_SUBINT_TXD1    4
#define S3C24XX_SUBINT_RXD1    3
#define S3C24XX_SUBINT_ERR0    2
#define S3C24XX_SUBINT_TXD0    1
#define S3C24XX_SUBINT_RXD0    0

static const UINT32 MAP_SUBINT_TO_INT[15] =
{
	S3C24XX_INT_UART0, S3C24XX_INT_UART0, S3C24XX_INT_UART0,
	S3C24XX_INT_UART1, S3C24XX_INT_UART1, S3C24XX_INT_UART1,
	S3C24XX_INT_UART2, S3C24XX_INT_UART2, S3C24XX_INT_UART2,
	S3C24XX_INT_ADC, S3C24XX_INT_ADC,
	S3C24XX_INT_CAM, S3C24XX_INT_CAM,
	S3C24XX_INT_WDT_AC97, S3C24XX_INT_WDT_AC97
};

#define S3C24XX_BPPMODE_STN_01		0x00
#define S3C24XX_BPPMODE_STN_02		0x01
#define S3C24XX_BPPMODE_STN_04		0x02
#define S3C24XX_BPPMODE_STN_08		0x03
#define S3C24XX_BPPMODE_STN_12_P	0x04
#define S3C24XX_BPPMODE_STN_12_U	0x05
#define S3C24XX_BPPMODE_STN_16		0x06
#define S3C24XX_BPPMODE_TFT_01		0x08
#define S3C24XX_BPPMODE_TFT_02		0x09
#define S3C24XX_BPPMODE_TFT_04		0x0A
#define S3C24XX_BPPMODE_TFT_08		0x0B
#define S3C24XX_BPPMODE_TFT_16		0x0C
#define S3C24XX_BPPMODE_TFT_24		0x0D

#define S3C24XX_PNRMODE_STN_04_DS  0
#define S3C24XX_PNRMODE_STN_04_SS  1
#define S3C24XX_PNRMODE_STN_08_SS  2
#define S3C24XX_PNRMODE_TFT        3

#define S3C24XX_GPIO_PORT_A S3C2440_GPIO_PORT_A
#define S3C24XX_GPIO_PORT_B S3C2440_GPIO_PORT_B
#define S3C24XX_GPIO_PORT_C S3C2440_GPIO_PORT_C
#define S3C24XX_GPIO_PORT_D S3C2440_GPIO_PORT_D
#define S3C24XX_GPIO_PORT_E S3C2440_GPIO_PORT_E
#define S3C24XX_GPIO_PORT_F S3C2440_GPIO_PORT_F
#define S3C24XX_GPIO_PORT_G S3C2440_GPIO_PORT_G
#define S3C24XX_GPIO_PORT_H S3C2440_GPIO_PORT_H
#define S3C24XX_GPIO_PORT_J S3C2440_GPIO_PORT_J

#define S3C24XX_CORE_PIN_NCON S3C2440_CORE_PIN_NCON
#define S3C24XX_CORE_PIN_OM0  S3C2440_CORE_PIN_OM0
#define S3C24XX_CORE_PIN_OM1  S3C2440_CORE_PIN_OM1

#define S3C24XX_UART_COUNT  3
#define S3C24XX_DMA_COUNT   4
#define S3C24XX_SPI_COUNT   2

/*******************************************************************************
    TYPE DEFINITIONS
*******************************************************************************/

typedef struct
{
	UINT32 data[0x34/4];
} s3c24xx_memcon_regs_t;

typedef struct
{
	UINT32 data[0x5C/4];
} s3c24xx_usbhost_regs_t;

typedef struct
{
	UINT32 srcpnd;
	UINT32 intmod;
	UINT32 intmsk;
	UINT32 priority;
	UINT32 intpnd;
	UINT32 intoffset;
	UINT32 subsrcpnd;
	UINT32 intsubmsk;
} s3c24xx_irq_regs_t;

typedef struct
{
	UINT32 disrc;
	UINT32 disrcc;
	UINT32 didst;
	UINT32 didstc;
	UINT32 dcon;
	UINT32 dstat;
	UINT32 dcsrc;
	UINT32 dcdst;
	UINT32 dmasktrig;
} s3c24xx_dma_regs_t;

typedef struct
{
	UINT32 locktime;
	UINT32 mpllcon;
	UINT32 upllcon;
	UINT32 clkcon;
	UINT32 clkslow;
	UINT32 clkdivn;
	UINT32 camdivn;
} s3c24xx_clkpow_regs_t;

typedef struct
{
	UINT32 lcdcon1;
	UINT32 lcdcon2;
	UINT32 lcdcon3;
	UINT32 lcdcon4;
	UINT32 lcdcon5;
	UINT32 lcdsaddr1;
	UINT32 lcdsaddr2;
	UINT32 lcdsaddr3;
	UINT32 redlut;
	UINT32 greenlut;
	UINT32 bluelut;
	UINT32 reserved[8];
	UINT32 dithmode;
	UINT32 tpal;
	UINT32 lcdintpnd;
	UINT32 lcdsrcpnd;
	UINT32 lcdintmsk;
	UINT32 tconsel;
} s3c24xx_lcd_regs_t;

typedef struct
{
	UINT32 data[0x400/4];
} s3c24xx_lcdpal_regs_t;

typedef struct
{
	UINT32 nfconf;
	UINT32 nfcont;
	UINT32 nfcmd;
	UINT32 nfaddr;
	UINT32 nfdata;
	UINT32 nfmeccd0;
	UINT32 nfmeccd1;
	UINT32 nfseccd;
	UINT32 nfstat;
	UINT32 nfestat0;
	UINT32 nfestat1;
	UINT32 nfmecc0;
	UINT32 nfmecc1;
	UINT32 nfsecc;
	UINT32 nfsblk;
	UINT32 nfeblk;
} s3c24xx_nand_regs_t;

typedef struct
{
	UINT32 data[0xA4/4];
} s3c24xx_cam_regs_t;

typedef struct
{
	UINT32 ulcon;
	UINT32 ucon;
	UINT32 ufcon;
	UINT32 umcon;
	UINT32 utrstat;
	UINT32 uerstat;
	UINT32 ufstat;
	UINT32 umstat;
	UINT32 utxh;
	UINT32 urxh;
	UINT32 ubrdiv;
} s3c24xx_uart_regs_t;

typedef struct
{
	UINT32 tcfg0;
	UINT32 tcfg1;
	UINT32 tcon;
	UINT32 tcntb0;
	UINT32 tcmpb0;
	UINT32 tcnto0;
	UINT32 tcntb1;
	UINT32 tcmpb1;
	UINT32 tcnto1;
	UINT32 tcntb2;
	UINT32 tcmpb2;
	UINT32 tcnto2;
	UINT32 tcntb3;
	UINT32 tcmpb3;
	UINT32 tcnto3;
	UINT32 tcntb4;
	UINT32 tcnto4;
} s3c24xx_pwm_regs_t;

typedef struct
{
	UINT32 data[0x130/4];
} s3c24xx_usbdev_regs_t;

typedef struct
{
	UINT32 wtcon;
	UINT32 wtdat;
	UINT32 wtcnt;
} s3c24xx_wdt_regs_t;

typedef struct
{
	UINT32 iiccon;
	UINT32 iicstat;
	UINT32 iicadd;
	UINT32 iicds;
	UINT32 iiclc;
} s3c24xx_iic_regs_t;

typedef struct
{
	UINT32 iiscon;
	UINT32 iismod;
	UINT32 iispsr;
	UINT32 iisfcon;
	UINT32 iisfifo;
} s3c24xx_iis_regs_t;

typedef struct
{
	UINT32 gpacon;
	UINT32 gpadat;
	UINT32 pad_08;
	UINT32 pad_0c;
	UINT32 gpbcon;
	UINT32 gpbdat;
	UINT32 gpbup;
	UINT32 pad_1c;
	UINT32 gpccon;
	UINT32 gpcdat;
	UINT32 gpcup;
	UINT32 pad_2c;
	UINT32 gpdcon;
	UINT32 gpddat;
	UINT32 gpdup;
	UINT32 pad_3c;
	UINT32 gpecon;
	UINT32 gpedat;
	UINT32 gpeup;
	UINT32 pad_4c;
	UINT32 gpfcon;
	UINT32 gpfdat;
	UINT32 gpfup;
	UINT32 pad_5c;
	UINT32 gpgcon;
	UINT32 gpgdat;
	UINT32 gpgup;
	UINT32 pad_6c;
	UINT32 gphcon;
	UINT32 gphdat;
	UINT32 gphup;
	UINT32 pad_7c;
	UINT32 misccr;
	UINT32 dclkcon;
	UINT32 extint0;
	UINT32 extint1;
	UINT32 extint2;
	UINT32 eintflt0;
	UINT32 eintflt1;
	UINT32 eintflt2;
	UINT32 eintflt3;
	UINT32 eintmask;
	UINT32 eintpend;
	UINT32 gstatus0;
	UINT32 gstatus1;
	UINT32 gstatus2;
	UINT32 gstatus3;
	UINT32 gstatus4;
	UINT32 pad_c0;
	UINT32 pad_c4;
	UINT32 pad_c8;
	UINT32 mslcon;
	UINT32 gpjcon;
	UINT32 gpjdat;
	UINT32 gpjup;
} s3c24xx_gpio_regs_t;

typedef struct
{
	UINT32 rtccon;
	UINT32 ticnt;
	UINT32 reserved[2];
	UINT32 rtcalm;
	UINT32 almsec;
	UINT32 almmin;
	UINT32 almhour;
	UINT32 almday;
	UINT32 almmon;
	UINT32 almyear;
	UINT32 rtcrst;
	UINT32 bcdsec;
	UINT32 bcdmin;
	UINT32 bcdhour;
	UINT32 bcdday;
	UINT32 bcddow;
	UINT32 bcdmon;
	UINT32 bcdyear;
} s3c24xx_rtc_regs_t;

typedef struct
{
	UINT32 adccon;
	UINT32 adctsc;
	UINT32 adcdly;
	UINT32 adcdat0;
	UINT32 adcdat1;
	UINT32 adcupdn;
} s3c24xx_adc_regs_t;

typedef struct
{
	UINT32 spcon;
	UINT32 spsta;
	UINT32 sppin;
	UINT32 sppre;
	UINT32 sptdat;
	UINT32 sprdat;
} s3c24xx_spi_regs_t;

typedef struct
{
	UINT32 data[0x44/4];
} s3c24xx_sdi_regs_t;

typedef struct
{
	UINT32 data[0x20/4];
} s3c24xx_ac97_regs_t;

typedef struct
{
	s3c24xx_memcon_regs_t regs;
} s3c24xx_memcon_t;

typedef struct
{
	s3c24xx_usbhost_regs_t regs;
} s3c24xx_usbhost_t;

typedef struct
{
	s3c24xx_irq_regs_t regs;
	int line_irq, line_fiq;
} s3c24xx_irq_t;

typedef struct
{
	s3c24xx_dma_regs_t regs;
	emu_timer *timer;
} s3c24xx_dma_t;

typedef struct
{
	s3c24xx_clkpow_regs_t regs;
} s3c24xx_clkpow_t;

typedef struct
{
	s3c24xx_lcd_regs_t regs;
	emu_timer *timer;
	bitmap_rgb32 *bitmap[2];
	UINT32 vramaddr_cur;
	UINT32 vramaddr_max;
	UINT32 offsize;
	UINT32 pagewidth_cur;
	UINT32 pagewidth_max;
	UINT32 bppmode;
	UINT32 bswp, hwswp;
	int vpos, hpos;
	double framerate;
	UINT32 tpal;
	UINT32 hpos_min, hpos_max, vpos_min, vpos_max;
	UINT32 dma_data, dma_bits;
} s3c24xx_lcd_t;

typedef struct
{
	s3c24xx_lcdpal_regs_t regs;
} s3c24xx_lcdpal_t;

typedef struct
{
	s3c24xx_nand_regs_t regs;
	UINT8 mecc[4];
	UINT8 secc[2];
	int ecc_pos, data_count;
} s3c24xx_nand_t;

typedef struct
{
	s3c24xx_cam_regs_t regs;
} s3c24xx_cam_t;

typedef struct
{
	s3c24xx_uart_regs_t regs;
} s3c24xx_uart_t;

typedef struct
{
	s3c24xx_pwm_regs_t regs;
	emu_timer *timer[5];
	UINT32 cnt[5];
	UINT32 cmp[5];
	UINT32 freq[5];
} s3c24xx_pwm_t;

typedef struct
{
	s3c24xx_usbdev_regs_t regs;
} s3c24xx_usbdev_t;

typedef struct
{
	s3c24xx_wdt_regs_t regs;
	emu_timer *timer;
} s3c24xx_wdt_t;

typedef struct
{
	s3c24xx_iic_regs_t regs;
	emu_timer *timer;
	int count;
} s3c24xx_iic_t;

typedef struct
{
	s3c24xx_iis_regs_t regs;
	emu_timer *timer;
	UINT16 fifo[16/2];
	int fifo_index;
} s3c24xx_iis_t;

typedef struct
{
	s3c24xx_gpio_regs_t regs;
} s3c24xx_gpio_t;

typedef struct
{
	s3c24xx_rtc_regs_t regs;
	emu_timer *timer_tick_count;
	emu_timer *timer_update;
} s3c24xx_rtc_t;

typedef struct
{
	s3c24xx_adc_regs_t regs;
} s3c24xx_adc_t;

typedef struct
{
	s3c24xx_spi_regs_t regs;
} s3c24xx_spi_t;

typedef struct
{
	s3c24xx_sdi_regs_t regs;
} s3c24xx_sdi_t;

typedef struct
{
	s3c24xx_ac97_regs_t regs;
} s3c24xx_ac97_t;

typedef struct
{
	const s3c2440_interface *iface;
	UINT8 steppingstone[4*1024];
	s3c24xx_memcon_t memcon;
	s3c24xx_usbhost_t usbhost;
	s3c24xx_irq_t irq;
	s3c24xx_dma_t dma[S3C24XX_DMA_COUNT];
	s3c24xx_clkpow_t clkpow;
	s3c24xx_lcd_t lcd;
	s3c24xx_lcdpal_t lcdpal;
	s3c24xx_nand_t nand;
	s3c24xx_cam_t cam;
	s3c24xx_uart_t uart[S3C24XX_UART_COUNT];
	s3c24xx_pwm_t pwm;
	s3c24xx_usbdev_t usbdev;
	s3c24xx_wdt_t wdt;
	s3c24xx_iic_t iic;
	s3c24xx_iis_t iis;
	s3c24xx_gpio_t gpio;
	s3c24xx_rtc_t rtc;
	s3c24xx_adc_t adc;
	s3c24xx_spi_t spi[S3C24XX_SPI_COUNT];
	s3c24xx_sdi_t sdi;
	s3c24xx_ac97_t ac97;
} s3c24xx_t;

#endif
