#ifndef __ISPFUSERESCUE_H_
#define __ISPFUSERESCUE_H_

//	FuseRescue
//	This sketch licensed under the MIT License (MIT)
//	Copyright (c) 2015 hieromon@gmail.com
//
// ArduinoISP version 04m3
// Copyright (c) 2008-2011 Randall Bohn
// If you require a license, see
//     http://www.opensource.org/licenses/bsd-license.php

// This sketch to aggregate two functions of the ArduinoISP and the FuseRescue.
// Enable either the FuseRecue or ArduinoISP by A0 signal.
// ArduinoISP sketch was captured by the .cpp file from the original code that
// is in the example of Arduino IDE and it was capsuled in the namespace as
// ArduinoISP. The original logic has not been modified.

#include "ArduinoISP.h"
#include "FuseRescue.h"

#define ACTMODE		A0

typedef enum {
	PCB_FUSERESCUE,
	PCB_ARDUINOISP
} PCBFUNC;

#endif	/* __ISPFUSERESCUE_H_ */
