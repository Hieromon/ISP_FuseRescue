#ifndef	__DEVICESIG_H__
#define	__DEVICESIG_H__

//	devicesig.h
//	This sketch licensed under the MIT License (MIT)
//	Copyright (c) 2015 hieromon@gmail.com
//	Declaration of type and device characteristics for the sketch of the fuse rescue shield

#include <avr/pgmspace.h>

// These declaration are the pins assignment for Arduino as the fuse programmer.
// The pin supplies a signal required for parallel programming to the chip
// to be programmed.
// This sketch turns the Arduino into the target chip using the following Arduino pins:
//		Symbol:		Define:		PGM:		PCB:	Target pins:
#define	VCC_ENABLE	7		//	VCC:		 D7:	VCC, AVCC
#define	PGM_ENABLE	13		//	+12V:		D13:	#RESET
#define	RDYBSY		12		//	RDY/#BSY:	D12:	PD1
#define	OE			11		//	#OE:		D11:	PD2
#define	WR			A0		//	#WR:		 A0:	PD3
#define	BS1			A1		//	BS1:		 A1:	PD4
#define	XA0			A3		//	XA0:		 A3:	PD5
#define	XA1			A4		//	XA1:		 A4:	PD6
#define	BS2			10		//	BS2:		D10:	PC2
#define	XTAL1		A2		//	XTAL1:		 A2:	XTAL1
#define	DATA0		A5		//	DATA0:		 A5:	PB0
#define	DATA1		2		//	DATA1:		 D2:	PB1
#define	DATA2		3		//	DATA2:		 D3:	PB2
#define	DATA3		4		//	DATA3:		 D4:	PB3
#define	DATA4		5		//	DATA4:		 D5:	PB4
#define	DATA5		6		//	DATA5:		 D6:	PB5
#define	DATA6		8		//	DATA6:		 D8:	PC0
#define	DATA7		9		//	DATA7:		 D9:	PBC
// The signal series input and output by sequenced data
const uint8_t	PGM_DATA[] = { DATA0, DATA1, DATA2, DATA3, DATA4, DATA5, DATA6, DATA7 };

// Parallel programming command codes
#define	CMD_CHIPERASE	B10000000			// Erase device
#define	CMD_WRITEFUSE	B01000000			// Write Fuse byte
#define	CMD_WRITELOCK	B00100000			// Write Lock byte
#define	CMD_READSIG		B00001000			// Read Signature
#define	CMD_READFUSE	B00000100			// Read Fuse byte

// Identifier indicating the byte for reading the fuse
typedef enum {
	_FUSE_BYTE_LOW,
	_FUSE_BYTE_HIGH,
	_FUSE_BYTE_EXT,
	_LOCK_BITS
} LOC_FUSE_BYTE;

// Container for the supported device characteristics
// Signature contains device signature byte
// A EEPROM size contains size of EEPROM
// Default fuse byte contains factory settings
typedef	struct	_device_sig {
	uint8_t		device[16];					// Chip name
	uint32_t	signature;					// signature byte
	uint16_t	eeprom_size;				// Size of EEPROM
	uint8_t		default_fuse[3];			// Chip default Fuse
	uint8_t		bt_fuse[3];					// Arduino bootloader Fuse
} device_sig_t;
// Device characteristics implementation
const device_sig_t DEVICE_LIST[] PROGMEM = {
	{"ATmega8"    , 0x1E9307,  256, {0xE1, 0xD9, 0xFF}, {0xE2, 0xDD, 0x77} },
	{"ATmega48A"  , 0x1E9205,  256, {0x62, 0xDF, 0xFF}, {0xE2, 0xDD, 0x77} },
	{"ATmega48PA" , 0x1E920A,  256, {0x62, 0xDF, 0xFF}, {0xE2, 0xDD, 0x77} },
	{"ATmega88A"  , 0x1E930A,  512, {0x62, 0xDF, 0xF9}, {0xE2, 0xDD, 0x77} },
	{"ATmega88PA" , 0x1E930F,  512, {0x62, 0xDF, 0xF9}, {0xE2, 0xDD, 0x77} },
	{"ATmega168A" , 0x1E9406,  512, {0x62, 0xDF, 0xF9}, {0xFF, 0xDD, 0x00} },
	{"ATmega168PA", 0x1E940B,  512, {0x62, 0xDF, 0xF9}, {0xFF, 0xDD, 0x00} },
	{"ATmega328"  , 0x1E9514, 1024, {0x62, 0xD9, 0xFF}, {0xFF, 0xDA, 0x05} },
	{"ATmega328P" , 0x1E950F, 1024, {0x62, 0xD9, 0xFF}, {0xFF, 0xDE, 0x05} }
};

// it would be held the UNKNOWN that the supported device could not be detected.
#define UNKNOWN_DEVICE	0xff

#endif	/* __DEVICESIG_H__ */
