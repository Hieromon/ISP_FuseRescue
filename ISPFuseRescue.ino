//	FuseRescue
//	This sketch licensed under the MIT License (MIT)
//	Copyright (c) 2015 hieromon@gmail.com
//
// ArduinoISP version 04m3
// Copyright (c) 2008-2011 Randall Bohn
// If you require a license, see
//     http://www.opensource.org/licenses/bsd-license.php
// 
// This sketch to aggregate two functions of the ArduinoISP and the FuseRescue.
// Enable either the FuseRecue or ArduinoISP by A0 signal.
// ArduinoISP sketch was captured by the .cpp file from the original code that
// is in the example of Arduino IDE and it was capsuled in the namespace as
// ArduinoISP. The original logic has not been modified.

#include <Arduino.h>
#include <MsTimer2.h>
#include "FuseRescue.h"
#include "ArduinoISP.h"
#include "ISPFuseRescue.h"

// Current function of PCB
static PCBFUNC	PCBMODE;

// Sensing the operational function either ArduinoISP or FuseRescue
PCBFUNC sense_pcb(void) {
	PCBFUNC	pcbSelection;

	// Power-up the target chip, the mode sense pin will be in non-dependent
	// from the target chip.
	pinMode(VCC_ENABLE, OUTPUT);
	digitalWrite(VCC_ENABLE, HIGH);
	// Wait for ACTMODE pin stable
	delay(1);

	// Detect PCB operation mode
	// ACTMODE pin is High, operation is FuseRescue
	// ACTMODE pin is Low, operation is Arduino ISP
	pinMode(ACTMODE, INPUT);
	pcbSelection = digitalRead(ACTMODE) == HIGH ? PCB_FUSERESCUE : PCB_ARDUINOISP;

	// Release the mode sense pin, turn off target chip.
	digitalWrite(VCC_ENABLE, LOW);
	// Stop completely the target chip waiting for discharge of the capacitor.
	delay(100);

	return pcbSelection;
}

void setup() {
	// Operation mode selection
	switch (PCBMODE = sense_pcb()) {
	case PCB_ARDUINOISP:
		ArduinoISP::setup();
		break;
	case PCB_FUSERESCUE:
		FuseRescue::setup();
		break;
	}
}

void loop() {
	switch (PCBMODE) {
	case PCB_ARDUINOISP:
		ArduinoISP::loop();
		break;
	case PCB_FUSERESCUE:
		FuseRescue::loop();
		break;
	}
}
