#ifndef	__FUSERESCUE_H_
#define	__FUSERESCUE_H_

//	FuseRescue.ino
//	This sketch licensed under the MIT License (MIT)
//	Copyright (c) 2015 hieromon@gmail.com

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include "devicesig.h"

namespace FuseRescue {
	// Retention of the current value for updating the fuse byte and byte lock
	// Device characteristic values
	// DEVICE_ID is index of the array which identified by the device signature
	// that currently targeted.
	extern uint8_t	DEVICE_ID;					// Device identify signature

	void	setup();							// Setup for the Sketch
	void	loop();								// Process of the Sketch
	char	inquiry(const char *, const char *, bool);	// Enter a query response
	int16_t	inquiry_hex(const char *);			// Enter hexadecimal value
	void	write_fuse_each(uint8_t);			// Write Fuse byte one by one
	void	write_fuse_default(void);			// Write default Fuse byte
	uint8_t	write_fuse(LOC_FUSE_BYTE, uint8_t);	// Write Fuse byte at a address
	void	write_lock_bits(void);				// Write Lock bits
	uint8_t	read_fuse(LOC_FUSE_BYTE);			// Read the Fuse byte at a address
	void	erase_device(void);					// Erase the Flash, Lock bits
	void	verify_device(void);				// Device verification
	uint32_t read_signature(void);				// Read the chip signature bytes
	void	stable_signals(void);				// Turn off programming signals
	void	setup_signals(void);				// Setup the each programming signal
	void	start_pgm(void);					// Launch parallel programming
	void	inline end_pgm(void);				// Parallel programing termination
	void	load_command(uint8_t);				// Release a command byte
	void	load_address_low(uint8_t);			// Discharge a address byte to data line
	void	load_data(uint8_t);					// Release a data byte
	uint8_t retrieve_data(void);				// Retrieve a data form the data line
	void	transmit_data(uint8_t);				// Latch a data
	void	persist_data(void);					// Write to memory for latched data
	// Time-out trapper declaration
	void	trap_timeout(uint8_t);			// Start timer for catch time-out
	void inline catch_timeout(void);		// Time-out trap routine
	void inline reset_timeout(void);		// Stop timer
};

#endif	/* __FUSERESCUE_H_ */
