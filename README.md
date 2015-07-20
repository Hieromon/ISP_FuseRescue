# ATmega88 High-voltage parallel fuse writer for Arduino

[“ú–{ŒêREADME_jp.md](https://github.com/Hieromon/ISP_FuseRescue/blob/master/README_jp.md "“ú–{ŒêREADME_jp.md")

![FuseRescue](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue.jpg)

## Introduction

The AVR mcu has the fuse byte but we do not see it at the arduino operation with the original state intact ordinarily. It to configure the operating state of the chip.  
If you are developing the AVR microcontroller in the native state with the ATMEL Studio, In rare cases it may inadvertently would rewrite the fuse bit. If its operation would rewrite the bits related to the reset vector for the clock source and the boot loader, the MCU cannot not be able to communicate with the outside by ISP or it is no longer able to be the ISP programming. At the Arduino IDE, it will take a error for the sketch writing such as below.

    avrdude: stk500_recv(): programmer is not responding

This error will appear when the boot loader written into the ATmega328 chip of Arduino does not respond. There are various causes. Even if the fuse bytes of ATmega328 is not consistent with the circuit of Arduino and the chip itself does not start, it will occur such errors. When it happen, it will not be able to return the original state of the chip by the serial programming method because the chip itself is locked. it cannot be written the boot loader.  
But the chip can be used if undo the fuse bytes because it not been destroyed yet. In order to do this you need to write an 8-bit parallel the fuse bytes while applying a voltage of +12V.

[ATmega88/168/328 datasheet](http://www.atmel.com/images/doc8161.pdf "ATmega48PA/88PA/168PA/328P - Complete(Revision D, 448 pages)") has described high-voltage parallel programming specification and the chip will be a good state by writing the correct value of the fuse in accordance with the procedure. To do this you must have some kind of circuit and software for writing the fuse. Tools for high voltage parallel programming of AVR microcontroller has been implemented by already various people, so it was published by [SatE-o](http://zampoh.cocolog-nifty.com/blog/2008/09/avr---atmega488.html) and [Yuki's lab.](http://yuki-lab.jp/hw/index.html).

**ATmega88 High-voltage parallel fuse writer for Arduino** is the shield and sketch for the high-voltage parallel fuse write.
* Supports ATmega88A/88PA/168A/168PA/328/328P 28pin DIP
* Arduino shield (required Arduino Uno)
* Interactively manipulating using the serial communication from the PC
* [Arduino as ISP](https://www.arduino.cc/en/Tutorial/ArduinoISP "Using an Arduino as an AVR ISP") also can be used as a writer shield (Switch the fuse writer and ISP writer by slide switch on the shield)

Also, it has the following features as a high-voltage parallel fuse writer.

* Fuse byte and extended fuse byte read / write
* Fuse byte write for Arduino boot loader
* Lock bit read / write
* Chip erase

## Schematic

![schematic](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/ISP_FuseRescue_C.sch.jpg)

The circuit is simple. It has the charge-pump to generate +12V from Arduino +5V for parallel programming, also it has [2-Ch analog multiplexer](http://www.ti.com/lit/ds/symlink/cd74hc4053.pdf "74HC4053") to select ISP writer or high-voltage programmer.  
The charge-pump [MC34063](http://www.onsemi.com/pub_link/Collateral/MC34063A-D.PDF "ON Semiconductor MC34063") is used but you can use the second source device as [NJM2360](http://www.njr.co.jp/products/semicon/products/NJM2360.html "JRC NJM2360").  
Gate selection terminal of analog multiplexer has led to +5V/GND selectable of the slide switch and the RESET of ATmega88 chip should be the target. When the gate +5V is selected, the all signal lines necessary to the parallel programming connect to Arduino and the target chip. And when the gate GND is selected, the signal lines for Arduino ISP connect to the target chip.

## Arduino sketch

It operates in the only one sketch if you want to use as Arduino as ISP writer also if you use as a high-voltage parallel programmer too. Arduino sketch of high-voltage parallel programmer consists of three modules.  

* ISPFuseRescue: The Main sketch
* FuseRescue: High voltage parallel programming sketch
* ArduinoISP: Arduino as ISP sketch (Example sketch itself that comes with the Arduino IDE)

ISPFuseRescue sketch would be stored into the sketch folder of Arduino, also FuseRescure and ArduinoISP stores to user library folder of Arduino.

## Usage

At first, you compile ISPFuseRescure and write to Arduino Uno that will be used to the writer. After that, set the high-voltage parallel fuse writer shield on the Arduino Uno and it connect with PC by USB serial.

![Arduino Operation](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue_arduino.jpg)

You start the serial terminal on the PC and connect to the arduino. Messages as the below on the terminal screen is displayed, the commands for programming will be able to input.

![Terminal Operation](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue_terminal.jpg)

### Commands

**L** : The lower fuse byte write
**H** : The upper fuse byte write
**X** : The extended fuse byte write
**K** : Lock bit write
**W** : Default (factory value) fuse byte write
**A** : Arduino fuse byte write
**E** : Chip erase
**V** : Verify the fuse byte or lock-bit

### Fuse byte and lock bits

#### ATmega88A/168A Extended Fuse Byte
Name|Bit No|Description|Defalut Value
----|------|-----------|-------------
- |7-3|-|1
BOOTSZ1|2|Select boot size|0
BOOTSZ0|1|Select boot size|0
BOOTRST|0|Select Reset Vector|1 (Application area)