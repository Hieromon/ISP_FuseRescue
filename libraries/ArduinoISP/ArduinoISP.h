#ifndef	__ARDUINOISP_H_
#define	__ARDUINOISP_H_

// ArduinoISP version 04m3
// Copyright (c) 2008-2011 Randall Bohn
// If you require a license, see
//     http://www.opensource.org/licenses/bsd-license.php

#include "Arduino.h"
#include "pins_arduino.h"

#define RESET     SS

#define LED_HB    9
#define LED_ERR   8
#define LED_PMODE 7
#define PROG_FLICKER true

#define HWVER 2
#define SWMAJ 1
#define SWMIN 18

// STK Definitions
#define STK_OK      0x10
#define STK_FAILED  0x11
#define STK_UNKNOWN 0x12
#define STK_INSYNC  0x14
#define STK_NOSYNC  0x15
#define CRC_EOP     0x20 //ok it is a space...

#define beget16(addr) (*addr * 256 + *(addr+1) )
typedef struct param {
	uint8_t	devicecode;
	uint8_t revision;
	uint8_t progtype;
	uint8_t parmode;
	uint8_t polling;
	uint8_t selftimed;
	uint8_t lockbytes;
	uint8_t fusebytes;
	int		flashpoll;
	int		eeprompoll;
	int		pagesize;
	int		eepromsize;
	int		flashsize;
} parameter;
#define PTIME 30

namespace ArduinoISP {
	extern int	error;
	extern int	pmode;
	extern int	here;						// address for reading and writing, set by 'U' command
	extern uint8_t	buff[];					// global block storage

	extern uint8_t	hbval;
	extern int8_t	hbdelta;

	void	pulse(int pin, int times);
	void	setup(void);
	void	heartbeat(void);
	void	loop(void);
	uint8_t	getch();
	void	fill(int n);
	void	prog_lamp(int state);
	void	spi_init();
	void	spi_wait();
	uint8_t	spi_send(uint8_t b);
	uint8_t	spi_transaction(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
	void	empty_reply();
	void	breply(uint8_t b);
	void	get_version(uint8_t c);
	void	set_parameters();
	void	start_pmode();
	void	end_pmode();
	void	universal();
	void	flash(uint8_t hilo, int addr, uint8_t data);
	void	commit(int addr);
	int		current_page(int addr);
	void	write_flash(int length);
	uint8_t	write_flash_pages(int length);
	uint8_t	write_eeprom(int length);
	uint8_t	write_eeprom_chunk(int start, int length);
	void	program_page();
	uint8_t	flash_read(uint8_t hilo, int addr);
	char	flash_read_page(int length);
	char	eeprom_read_page(int length);
	void	read_page();
	void	read_signature();
	int		avrisp();
};

#endif	/* __ARDUINOISP_H_ */
