//	FuseRescue.ino
//	This sketch licensed under the MIT License (MIT)
//	Copyright (c) 2015 hieromon@gmail.com
//
//	The sketch rewrites the device fuse in accordance with the parallel
//	programming.  The parallel programming method is described
//	the ATmega88 data sheet that is published from ATMEL.
//	This sketch turns the Arduino into the target chip using the following Arduino pins:
//		pin name:	uno:	target chip(mega88)
//		+12V:		D13:	#RESET
//		VCC:		 D7:	VCC, AVCC
//		RDY/#BSY:	D12:	PD1
//		#OE:		D11:	PD2
//		#WR:		 A0:	PD3
//		BS1:		 A1:	PD4
//		XA0:		 A3:	PD5
//		XA1:		 A4:	PD6
//		PAGEL:		N/A:	GND(via 10K ohm)
//		BS2:		D10:	PC2
//		XTAL1:		 A2:	XTAL1
//		DATA0:		 A5:	PB0
//		DATA1-5:	D2-6:	PB1-5
//		DATA6-7:	D8-9:	PC0-1

// INCLUDE directive dependency
#include "FuseRescue.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <MsTimer2.h>

// Embedded version string for the Sketch
#define VERSION		"0.9"

// Retention of the current value for updating the fuse byte and byte lock
// Device characteristic values
uint8_t	FuseRescue::DEVICE_ID;							// Device identify signature
static const device_sig_t	*DEVICE_TAG;			// Device identify tag

// Operation commands definition
#define OPCMD_WR_FUSE_LO	'L'				// Write low Fuse byte
#define OPCMD_WR_FUSE_HI	'H'				// Write high Fuse byte
#define OPCMD_WR_FUSE_EX	'X'				// Write extended Fuse byte
#define OPCMD_WR_FUSE_KY	'K'				// Write Lock bits
#define OPCMD_WR_FUSE_DE	'W'				// Write the default Fuse byte and lock byte
#define OPCMD_WR_FUSE_AR	'A'				// Write Arduino bootloader Fuse byte
#define OPCMD_ERASE			'E'				// Erase device
#define OPCMD_VERIFY		'V'				// Verify device
static const char	CMD_LEXDEFINE[] __attribute__ ((aligned)) = {
	OPCMD_WR_FUSE_LO, OPCMD_WR_FUSE_HI, OPCMD_WR_FUSE_EX,
	OPCMD_WR_FUSE_KY, OPCMD_WR_FUSE_DE, OPCMD_WR_FUSE_AR,
	OPCMD_ERASE, OPCMD_VERIFY, 0x00	
};

// Current executed command
static uint8_t	CMD_CURRENT;
// Command execution time out measure
volatile bool	CMD_TIMEOUT;				// Time-out occurrence
#define OPCMD_TRAP_TIMEOUT	200				// Time-out limit 200ms
#define OPCMD_RETRY_MAX		3				// Write command retry maximum count

// Create a FILE structure to reference for UART output function
static FILE	UART_OUT = { 0 };
// printf hooking up for Serial.write, although of type virtual,
// already exists.
static int	uart_putchar(char c, FILE *stream) {
	Serial.write(c);
	return 0;
}

/**
 * Declare time-out trap to sense RDY/#BSY signal lost.
 * If time-out occurs, global value as CMD_TIMEOUT would be indicated and
 * the function as 'catch_timeout' is trapper.
 * @param	milliseconds	limitation for sensing time-out by milliseconds unit
 */
void inline FuseRescue::catch_timeout(void) {
	CMD_TIMEOUT = true;
}

void inline FuseRescue::reset_timeout(void) {
	MsTimer2::stop();
}

void FuseRescue::trap_timeout(uint8_t milliseconds) {
	CMD_TIMEOUT = false;
	MsTimer2::set(milliseconds, catch_timeout);
	MsTimer2::start();
}

/**
 * Main Sketch code
 */
// Setup for the Sketch
void FuseRescue::setup() {
	// Control signals to the initial state
	stable_signals();

	// Start the UART
	Serial.begin(9600);
	// Fill in the UART file descriptor with pointer to writer.
	fdev_setup_stream(&UART_OUT, uart_putchar, NULL, _FDEV_SETUP_WRITE);
	// The uart is the standard output device STDOUT.
	stdout = &UART_OUT;

	// Start fuse operation via interactivity work
	printf_P(PSTR("High-voltage Fuse Rescue for ATmega88 Ver."VERSION"\r\n"));
	// Show the verified device
	CMD_CURRENT = OPCMD_VERIFY;
	verify_device();
}

// Process of the Sketch
void FuseRescue::loop() {
	uint8_t	command;

	if (CMD_CURRENT != 0x00) {
		// Show command list
		printf_P(PSTR("\r\n_________________________________________________\r\n"));
		if (DEVICE_ID != UNKNOWN_DEVICE) {
			printf_P(PSTR("%c:Write low Fuse byte\r\n"), OPCMD_WR_FUSE_LO);
			printf_P(PSTR("%c:Write high Fuse byte\r\n"), OPCMD_WR_FUSE_HI);
			printf_P(PSTR("%c:Write extended Fuse byte\r\n"), OPCMD_WR_FUSE_EX);
			printf_P(PSTR("%c:Write Lock bits\r\n"), OPCMD_WR_FUSE_KY);
			printf_P(PSTR("%c:Write default Fuse bytes {0x%02X,0x%02X,0x%02X}\r\n"), OPCMD_WR_FUSE_DE,
				pgm_read_byte(&DEVICE_TAG->default_fuse[0]),
				pgm_read_byte(&DEVICE_TAG->default_fuse[1]),
				pgm_read_byte(&DEVICE_TAG->default_fuse[2]));
			printf_P(PSTR("%c:Write Fuse bytes for Arduino bootloader\r\n"), OPCMD_WR_FUSE_AR);
			printf_P(PSTR("%c:Erase device\r\n"), OPCMD_ERASE);
		}
		printf_P(PSTR("%c:Verify device\r\n"), OPCMD_VERIFY);
	}

	// Scan the operation command from the serial port
	while ((command = (uint8_t)inquiry("\r\nEnter command -->", CMD_LEXDEFINE, false)) == 0x00);

	// Parse the command and dispatch the writing process
	CMD_CURRENT = command;
	Serial.println();
	if (DEVICE_ID == UNKNOWN_DEVICE)
		if (command != OPCMD_VERIFY) {
			printf_P(PSTR("'%c' command is not available now.\n\r"), command);
			CMD_CURRENT = 0x00;
			return;
		}
	switch (command) {
	case OPCMD_WR_FUSE_LO:
	case OPCMD_WR_FUSE_HI:
	case OPCMD_WR_FUSE_EX:
		write_fuse_each(command);
		break;
	case OPCMD_WR_FUSE_DE:
	case OPCMD_WR_FUSE_AR:
		write_fuse_default();
		break;
	case OPCMD_WR_FUSE_KY:
		write_lock_bits();
		break;
	case OPCMD_ERASE:
		erase_device();
		break;
	case OPCMD_VERIFY:
		verify_device();
		break;
	default:
		printf_P(PSTR("'%c': Unknown\r\n"), command);
		CMD_CURRENT = 0x00;
		break;
	}
}

/**
 * Enter a query response via serial with inquiry string display. 
 * Input termination is controled by momently flag. If {@code momently} flag is active,
 * input process will terminate at one character without ENTER waiting.
 * @param	query_string	A string of inquiry display
 * @param	mask			List of characters that are allowed to reply
 * @param	momently		The flag of {@code true} or {@code false}
 * @return	A response character
 */ 
char FuseRescue::inquiry(const char *query_string, const char *mask, bool momently) {
	char	c, rc = 0x00;
	uint8_t	c_count = 0;

	Serial.print(query_string);
	do {
		while (!Serial.available());
		c = Serial.read();
		if (isalpha(c))
			 c &= 0xdf;
		if (c == '\b') {
			if (c_count > 0) {
				Serial.print(c);
				rc = 0x00;
				c_count--;
			}
		} else if (strchr(mask, (int)c) > 0) {
			Serial.print(c);
			c_count++;
			rc = c;
			if (momently)
				break;
		} else if (c == '\n' || c == '\r')
			Serial.println();
		else
			Serial.print('\a');
	} while (c != '\n' && c != '\r');

	return rc;
}

/**
 * Enter hexadecimal value with inquiry string display that is restricted
 * input character as 0-9, A-F.
 * @param	query_string	
 * @return	A binary value from input hexadecimal character notation
 */
int16_t FuseRescue::inquiry_hex(const char *query_string) {
	char	bin;
	uint8_t	hex_value;

	bin = inquiry(query_string, "0123456789ABCEDF", true);
	if (bin == 0x00)
		return -1;
	hex_value += (bin >= '0' && bin <= '9') ? bin & 0x0f : bin - 0x37;

	bin = inquiry("", "0123456789ABCEDF", false);
	if (bin > 0x00) {
		hex_value <<= 4;
		hex_value += (bin >= '0' && bin <= '9') ? bin & 0x0f : bin - 0x37;
	}

	return (int16_t)hex_value;
}

/**
 * Write Fuse byte one by one as the extended byte, the low byte and the high byte.
 * @param	command		enumeration value
 */
void FuseRescue::write_fuse_each(uint8_t command) {
	LOC_FUSE_BYTE	fb;
	uint8_t			fuse;
	int16_t			new_fuse;
	const char		*loc_fuse;

	// Verify the current fuse byte and inquiry the write execution
	switch (command) {
	case OPCMD_WR_FUSE_LO:
		fb = _FUSE_BYTE_LOW;
		loc_fuse = "low";
		break;
	case OPCMD_WR_FUSE_HI:
		fb = _FUSE_BYTE_HIGH;
		loc_fuse = "high";
		break;
	case OPCMD_WR_FUSE_EX:
		fb = _FUSE_BYTE_EXT;
		loc_fuse = "ext";
		break;
	}
	// Read current Fuse byte which is specified and echo to display.
	fuse = read_fuse(fb);
	printf_P(PSTR("Current Fuse(%s) 0x%02X "), loc_fuse, fuse);
	// Inquiry the new Fuse value and execute writing
	if ((new_fuse = inquiry_hex(", Enter new value (HEX, NULL leave w/o change) --> ")) >= 0) {
		// New Fuse byte is specified, inquiry the writing
		fuse = (uint8_t)new_fuse;
		printf_P(PSTR("    New Fuse(%s) 0x%02X"), loc_fuse, fuse);
		if ((inquiry(".  Write ? (Y/N) ", "YN", false) & 0xdf) == 'Y') {
			// Execute write
			printf_P(PSTR("  Writing... "));
			uint8_t wb_fuse = write_fuse(fb, fuse);
			// If time-out with retry count excess occurs CMD_TIMEOUT will be indicated it.
			// If verify error occurs, value of the r_try will exceed limit.
			if (CMD_TIMEOUT)
				printf_P(PSTR("Time out, Fuse can not be written."));
			else if (wb_fuse != fuse)
				printf_P(PSTR("Verify 0x%0X"), wb_fuse);
			else
				printf_P(PSTR("complete."));
		}
	}
}

/**
 * Write default Fuse byte of each device which described in devicesig.h
 */
void FuseRescue::write_fuse_default(void) {
	LOC_FUSE_BYTE	fb[] = { _FUSE_BYTE_LOW, _FUSE_BYTE_HIGH, _FUSE_BYTE_EXT };
	const char	*fb_loc[] = { "low", "high", "ext" };

	if ((inquiry("Write default Fuse bytes ? (Y/N) ", "YN", false) & 0xdf) == 'Y') {
		uint8_t fuse, wb_fuse;
		// Inquiry Fuse writing, apply default Fuse bytes
		printf_P(PSTR("  Writing... "));
		for (uint8_t i = 0; i < sizeof fb / sizeof(LOC_FUSE_BYTE); i++) {
			// Retrieve default Fuse byte
			switch (CMD_CURRENT) {
			case  OPCMD_WR_FUSE_DE:
				fuse = pgm_read_byte(&DEVICE_TAG->default_fuse[i]);
				break;
			case  OPCMD_WR_FUSE_AR:
				fuse = pgm_read_byte(&DEVICE_TAG->bt_fuse[i]);
				break;
			}
			printf_P(PSTR("%s:0x%02X "), fb_loc[i], fuse);
			// Execute write
			uint8_t wb_fuse = write_fuse(fb[i], fuse);
			// If time-out with retry count excess occurs CMD_TIMEOUT will be indicated it.
			// If verify error occurs, value of the r_try will exceed limit.
			if (CMD_TIMEOUT) {
				printf_P(PSTR("Time out, Fuse can not be written."));
				break;
			} else if (wb_fuse != fuse) {
				printf_P(PSTR("Verify 0x%0X"), wb_fuse);
				break;
			}
		}
		if (!CMD_TIMEOUT & (wb_fuse == fuse))
			printf_P(PSTR(" complete."));
	}
}

/**
 * Write Fuse byte at address specified by loc parameter and when time-out
 * occurred at write sequence with monitoring RDY/\BSY signal,
 * it will be set CMD_TIMEOUT.
 * This write sequence will verify the wrote Fuse data, If verified Fuse is not
 * equal writing data then retry write again until OPCMD_RETRY_MAX.
 * @param	loc		Enumeration value of Fuse byte address as {@code _FUSE_BYTE_LOW},
 *					{@code _FUSE_BYTE_HIGH} and {@code _FUSE_BYTE_EXT}
 * @param	fuse	Fuse byte value should be written
 * @return	Verified Fuse data after writing finish
 */
uint8_t	FuseRescue::write_fuse(LOC_FUSE_BYTE loc, uint8_t fuse) {
	uint8_t		wb_fuse, r_try = 0;

	do {
		// Start the parallel programming sequence
		// that is attempted up to OPCMD_RETRY_MAX.
		delay(9);
		start_pgm();
		load_command(CMD_WRITEFUSE);		// load write fuse command
		load_data(fuse);					// Enables data loading
		// Specify Fuse byte location
		switch (loc) {
		case _FUSE_BYTE_LOW:				// Low byte of the fuse
			// The previous signal is available and the signal change does not necessary.
			//digitalWrite(BS1, LOW);
			//digitalWrite(BS2, LOW);
			break;
		case _FUSE_BYTE_HIGH:				// High byte of the fuse
			digitalWrite(BS1, HIGH);
			//digitalWrite(BS2, LOW);
			break;
		case _FUSE_BYTE_EXT:				// Extended byte of the fuse
			//digitalWrite(BS1, LOW);			// Stabilized
			digitalWrite(BS2, HIGH);
			break;
		}
		// Execute fuse writing
		persist_data();
		// Terminate the Fuse Writing
		digitalWrite(BS1, LOW);
		digitalWrite(BS2, LOW);
		end_pgm();
	} while (!CMD_TIMEOUT &&
			(((wb_fuse = read_fuse(loc)) != fuse) & (++r_try <= OPCMD_RETRY_MAX)));

	delay(100);
	return wb_fuse;
}

/**
 * Write Lock bits
 */
void FuseRescue::write_lock_bits(void) {
	uint8_t	lock_bits;
	int16_t	new_lock;

	lock_bits = read_fuse(_LOCK_BITS);
	printf_P(PSTR("Current Lock bits 0x%02X"), lock_bits);
	if ((new_lock = inquiry_hex(", Enter new value (HEX, NULL leave w/o change) --> ")) >= 0) {
		// New Lock byte is specified correctly, inquiry the writing
		lock_bits = (uint8_t)new_lock;
		printf_P(PSTR("    New Lock bits:0x%02X"), lock_bits);
		// Caution, if the combination of Lock bits apply with LB3, Fuse bytes
		// would be protected and it can not be changed in parallel programming.
		if (!(lock_bits & 0x03))
			printf_P(PSTR("(LB mode 3, Lock bits and Fuse bytes will be locked!)"));
		// Execute Lock bits writing after the inquiry
		if ((inquiry(".  Write ? (Y/N) ", "YN", false) & 0xdf) == 'Y') {
			printf_P(PSTR("  Writing... "));
			// Execute write
			uint8_t wb_lb = write_fuse(_LOCK_BITS, lock_bits);
			// If time-out with retry count excess occurs CMD_TIMEOUT will be indicated it.
			if (CMD_TIMEOUT)
				printf_P(PSTR("Time out, Lock bits can not be written."));
			else if (wb_lb != lock_bits)
				printf_P(PSTR("Verify 0x%02X"), wb_lb);
			else
				printf_P(PSTR("complete."));
		}
	}
}

	
/**
 * Read the Fuse byte at specified address by loc parameter as enumeration
 * for {@code _FUSE_BYTE_LOW}, {@code _FUSE_BYTE_HIGH} and {@code _FUSE_BYTE_EXT}.
 * @param	loc		Enumeration value of Fuse byte address as {@code _FUSE_BYTE_LOW},
 *					{@code _FUSE_BYTE_HIGH} and {@code _FUSE_BYTE_EXT}
 * @return	A Fuse byte value
 */
uint8_t FuseRescue::read_fuse(LOC_FUSE_BYTE loc) {
	uint8_t	fuse;
	
	// Start parallel programming with fuse read
	start_pgm();
	load_command(CMD_READFUSE);
	// Read each the fuse byte individually via loc parameter
	switch (loc) {
	case _FUSE_BYTE_LOW:
		// This line already has actual strobe,
		// the signal change is not necessary
		// digitalWrite(BS1, LOW);
		// digitalWrite(BS2, LOW);
		break;
	case _FUSE_BYTE_HIGH:
		digitalWrite(BS1, HIGH);
		digitalWrite(BS2, HIGH);
		break;
	case _FUSE_BYTE_EXT:
		// digitalWrite(BS1, LOW);				// No need change
		digitalWrite(BS2, HIGH);
		break;
	case _LOCK_BITS:
		digitalWrite(BS1, HIGH);
		// digitalWrite(BS2, LOW);				// No need change
		break;
	}
	// Stroke the output enable, read fuse byte
	fuse = retrieve_data();
	// Stop programming
	end_pgm();

	return fuse;
}

/**
 * Erase the Flash and EEPROM memories plus Lock bits
 */
void FuseRescue::erase_device(void) {
	if ((inquiry("Flash and EEPROM, Lock bits will be cleared. Erase ? (Y/N) ", "YN", false) & 0xdf) == 'Y') {
		printf_P(PSTR("Erasing... "));
		uint8_t	r_try = 0;
		do {
			// Start the parallel programming sequence
			// that is attempted up to OPCMD_RETRY_MAX.
			delay(9);
			start_pgm();
			load_command(CMD_CHIPERASE);		// Load erase command
			// Execute chip erase
			persist_data();
			end_pgm();							// End parallel programming
		} while (CMD_TIMEOUT && ++r_try < OPCMD_RETRY_MAX);
		// If time-out with retry count excess occurs CMD_TIMEOUT will be indicated it.
		printf_P(CMD_TIMEOUT ? PSTR("Time out, chip can not be erased.") : PSTR("complete."));
	}
}

/**
 * Read signature byte from the target chip and echo the device name identified
 * by the read signature. Global variable DEVICE_TAG will have index of device
 * characteristics table identified by the signature. DEVICE_TAG is consulted
 * from the other functions when it will read the default Fuse bytes etc.
 */
void FuseRescue::verify_device(void) {
	uint32_t	detect_sig;
	uint8_t		vf_fuse_low, vf_fuse_high, vf_fuse_ext, vf_lock;

	// Show the verified device
	DEVICE_ID = UNKNOWN_DEVICE;
	printf_P(PSTR("  Verify the target... "));
	detect_sig = read_signature();
	for (uint8_t detect_id = 0; detect_id < sizeof DEVICE_LIST / sizeof(device_sig_t); detect_id++) {
		DEVICE_TAG = &DEVICE_LIST[detect_id];
		if (detect_sig == (uint32_t)pgm_read_dword(&DEVICE_TAG->signature)) {
			DEVICE_ID = detect_id;
			break;
		}
	}
	// DEVICE_ID still unknown, not supported device detected
	if (DEVICE_ID == UNKNOWN_DEVICE)
		printf_P(PSTR("Signature:0x%06lX  UNKNOWN DEVICE\r\n"), detect_sig);
	else {
		// Detects the correct device
		// Echo device type from device characteristics table
		uint8_t		p;
		const uint8_t	*p_buffer = DEVICE_TAG->device;
		while ((p = pgm_read_byte(p_buffer++)))
			Serial.write(p);
		// Inquiry current fuse byte and lock byte
		vf_fuse_low = read_fuse(_FUSE_BYTE_LOW);
		vf_fuse_high = read_fuse(_FUSE_BYTE_HIGH);
		vf_fuse_ext = read_fuse(_FUSE_BYTE_EXT);
		vf_lock = read_fuse(_LOCK_BITS);
		// Responds current byte value
		printf_P(PSTR("(0x%06lX)\n\r  Fuse:0x%02X(low),0x%02X(high),0x%02X(ext)  Lock:0x%02X\n\r"),
			detect_sig, vf_fuse_low, vf_fuse_high, vf_fuse_ext, vf_lock);
	}
}

/**
 * Read the chip signature bytes.
 * Although the signature of device is usually 3 bytes, this function returns
 * the signature three consecutive bytes as a long integer.
 * @return	A read signature by long integer
 */
uint32_t FuseRescue::read_signature(void) {
	uint32_t	signature = 0L;

	// Start parallel programming
	start_pgm();
	// Release the read signature command
	load_command(CMD_READSIG);
	// Capture the signature three consecutive bytes
	for (uint8_t address = 0x00; address <= 0x02; address++) {
		load_address_low(address);
		signature = (signature << 8) + retrieve_data();
	}
	// Stop programming
	end_pgm();

	return signature;
}

/**
 * Turn off programming signals for the signals stability.
 * All signals change input mode to the signals idle status.
 */
void FuseRescue::stable_signals(void) {
	// Deactivate whole signals
	digitalWrite(VCC_ENABLE, LOW);
	digitalWrite(PGM_ENABLE, LOW);
	pinMode(VCC_ENABLE, OUTPUT);
	pinMode(PGM_ENABLE, OUTPUT);
	pinMode(RDYBSY, INPUT);
	pinMode(OE, INPUT);
	pinMode(WR, INPUT);
	pinMode(BS1, INPUT);
	pinMode(BS2, INPUT);
	pinMode(XA0, INPUT);
	pinMode(XA1, INPUT);
	pinMode(XTAL1, INPUT);
	for (uint8_t i = 0; i < 8; i++)
		pinMode(PGM_DATA[i], INPUT);
}

/**
 * Setup the each signal for launch the parallel programming
 * These signal transition is necessary for correct activation of
 * the parallel programming.
 */
void FuseRescue::setup_signals(void) {
	// Start up: VCC off, +12V turn off, XTAL1 stillness
	// Deactivate read/write signals
	pinMode(WR, OUTPUT);
	digitalWrite(WR, HIGH);
	pinMode(OE, OUTPUT);
	digitalWrite(OE, HIGH);
	// Prepare the Prog_enable pins
	// XA0 & XA1 stop, BS1 & BS2 stillness
	digitalWrite(XA1, LOW);
	pinMode(XA1, OUTPUT);
	digitalWrite(XA0, LOW);
	pinMode(XA0, OUTPUT);
	digitalWrite(BS1, LOW);
	pinMode(BS1, OUTPUT);
	digitalWrite(BS2, LOW);
	pinMode(BS2, OUTPUT);
	// XTAL1 stillness
	digitalWrite(XTAL1, LOW);
	pinMode(XTAL1, OUTPUT);
	// RDY/#BSY
	pinMode(RDYBSY, OUTPUT);
	digitalWrite(RDYBSY, LOW);
}

/**
 * Launch parallel programming
 */
void FuseRescue::start_pgm(void) {
	setup_signals();
	// Enter the parallel programming mode
	// VCC on, apply +12V
	digitalWrite(VCC_ENABLE, HIGH);
	delayMicroseconds(30);
	digitalWrite(PGM_ENABLE, HIGH);
	// Wait until the + 12V supply is sufficiently
	// Launch the parallel programming sequence
	delayMicroseconds(10);
	pinMode(RDYBSY, INPUT);
	delayMicroseconds(300);
}

/**
 * Parallel programing termination
 */
void inline FuseRescue::end_pgm(void) {
	// Exit the parallel programming mode
	stable_signals();
}

/**
 * Release a command byte
 * @param	command		A command byte
 */
void FuseRescue::load_command(uint8_t command) {
	digitalWrite(XA1, HIGH);
	digitalWrite(XA0, LOW);
	digitalWrite(BS1, LOW);
	transmit_data(command);
}

/**
 * Discharge a address byte to data line
 * @param	address		A address byte
 */
void FuseRescue::load_address_low(uint8_t address) {
	digitalWrite(XA1, LOW);
	digitalWrite(XA0, LOW);
	digitalWrite(BS1, LOW);
	transmit_data(address);
}

/**
 * Release a data byte
 * @param	data		data byte
 */
void FuseRescue::load_data(uint8_t data) {
	digitalWrite(XA1, LOW);
	digitalWrite(XA0, HIGH);
	digitalWrite(BS1, LOW);
	transmit_data(data);
}

/**
 * Retrieve a data form the data line
 * @return	A data on the data line
 */
uint8_t FuseRescue::retrieve_data(void) {
	uint8_t	read_byte = 0x00;
	int8_t	bit_count;

	// Receive the discharged 8 bit data on the line
	digitalWrite(OE, LOW);
	// Scan the data line and concatenate it to the byte
	for (bit_count = 7; bit_count >= 0; bit_count--) {
		read_byte <<= 1;
		read_byte &= 0xFE;
		pinMode(PGM_DATA[bit_count], INPUT);
		if (digitalRead(PGM_DATA[bit_count]) == HIGH)
			read_byte |= 0x01;
	}
	// Close the data line
	digitalWrite(OE, HIGH);
	delay(1);

	return read_byte;
}

/**
 * Latch a data
 * @param	data	A data to latch
 */
void FuseRescue::transmit_data(uint8_t data) {
	// Change signal direction for data line and release data
	for (uint8_t bit_count = 0; bit_count < 8; bit_count++) {
		pinMode(PGM_DATA[bit_count], OUTPUT);
		digitalWrite(PGM_DATA[bit_count], data & 0x01);
		data >>= 1;
	}
	// Give XTAL1 a positive pulse, this loads the command.
	digitalWrite(XTAL1, HIGH);
	delayMicroseconds(1);
	digitalWrite(XTAL1, LOW);
	delayMicroseconds(1);
}

/**
 * Write to memory for latched data
 */
void FuseRescue::persist_data(void) {
	// Execute #WR pulse to persist a sent data
	trap_timeout(OPCMD_TRAP_TIMEOUT);	// Set time-out trap
	digitalWrite(WR, LOW);				// Signal the writing pulse
	digitalWrite(WR, HIGH);
	delayMicroseconds(1);				// Stabilize the write completion
	while (!digitalRead(RDYBSY));		// Waiting for erasing completely
	reset_timeout();					// Release the trap for time-out
}