# ATmega88 High-voltage parallel fuse writer for Arduino

[日本語README_jp.md](https://github.com/Hieromon/ISP_FuseRescue/blob/master/README_jp.md "日本語README_jp.md")

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

#### ATmega88A/168A Fuse High Byte
Name|Bit No|Description|Defalut Value
----|------|-----------|-------------
RSTDISBL|7|External Reset Disable|1 (RESET)
DWEN|6|debugWIRE Enable|1 (Disable)
SPIEN|5|Enable Serial Program and Data Downloading|0 (SPI programming enabled)
WDTON|4|Watchdog Timer Always On|1 (On at WDTCSR)
EESAVE|3|EEPROM memory is preserved through the Chip Erase|1 (EEPROM not reserved)
BODLEVEL2|2|Brown-out Detector trigger level|1
BODLEVEL1|1|Brown-out Detector trigger level|1
BODLEVEL0|0Brown-out Detector trigger level|1

#### ATmega328 Extended Fuse Byte
Name|Bit No|Description|Defalut Value
----|------|-----------|-------------
- |7～3|-|1
BODLEVEL2|2|Brown-out Detector trigger level|1
BODLEVEL1|1|Brown-out Detector trigger level|1
BODLEVEL0|0|Brown-out Detector trigger level|1

#### ATmega328 Fuse HIgh Byte
Name|Bit No|Description|Defalut Value
----|------|-----------|-------------
RSTDISBL|7|External Reset Disable|1 (RESET)
DWEN|6|debugWIRE Enable|1 (Disable)
SPIEN|5|Enable Serial Program and Data Downloading|0 (SPI programming enabled)
WDTON|4|Watchdog Timer Always On|1 (On at WDTCSR)
EESAVE|3|EEPROM memory is preserved through the Chip Erase|1 (EEPROM not reserved)
BOOTSZ1|2|Select Boot Size|0
BOOTSZ0|1|Select Boot Size|0
BOOTRST|0|Select Reset Vector|1 (Application area)

#### Fuse Low Byte
Name|Bit No|Description|Defalut Value
----|------|-----------|-------------
CKDIV8|7|Divide clock by 8|0 (Yes)
CKOUT|6|Clock output|1 (Disabled)
SUT1|5|Select start-up time|1
SUT2|4|Select start-up time|0
CKSEL3|3|Select Clock source|0
CKSEL2|2|Select Clock source|0
CKSEL1|1|Select Clock source|1
CKSEL0|0|Select Clock source|0

#### Lock Bits
Name|Bit No|Description|Defalut Value
----|------|-----------|-------------
- |7-6|-|1|
BLB12|5|Boot Lock bit|1|
BLB11|4|Boot Lock bit|1|
BLB02|3|Boot Lock bit|1|
BLB01|2|Boot Lock bit|1|
LB2|1|Lock bit|1|
LB1|0|Lock bit|1|

#### The fuse byte for Arduino
If you want ATmega328 behavior as Arduino, you will need to specify a clock source and a boot loader in the fuse bytes. Its value has been described in the `<Arduino system folder>\hardware\arduino\avr\boards.txt` file. (Path in the case of Arduino IDE 1.6.0)
````
uno.bootloader.low_fuses=0xFF
uno.bootloader.high_fuses=0xDE
uno.bootloader.extended_fuses=0x05
uno.bootloader.unlock_bits=0x3F
uno.bootloader.lock_bits=0x0F
````

### Caution!!!

You can not operate the switch for changing High-voltage parallel fuse writer and the ISP writer while connect with PC by USB. It is short-circuited. Changing operation must be in the state where power is not supplied.

## Appendix

#### Eagle Cad files(Schematic and PCB layout)

For reference, the PCB data of Eagle cad will be posted. This PCB is layouted with SMD parts.

[Eagel CAD file](https://github.com/Hieromon/ISP_FuseRescue/blob/master/ISP_FuseRescue_C.zip "Eagle CAD file") (zip)

![PCB\_LAYOUT](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue_pcblayout.jpg)
![PCB](https://raw.github.com/wiki/hieromon/ISP_FuseRescue/images/FuseRescue_pcb.jpg)

#### BOM 
Part|Value|Device|Package
----|-----|------|-------
C1|330μ|Aluminum Electrolytic Capacitor|SMD Φ6.3mm x 7.7mm
C2|100μ|Aluminum Electrolytic Capacitor|SMD Φ8.0mm x 10.2mm
C3|680pF|Ceramic Capacitor|2012 SMD
C4|0.1μF|Ceramic Capacitor|2012 SMD
C5|22pF|Ceramic Capacitor|1608 SMD
C6|22pF|Ceramic Capacitor|1608 SMD
C7|0.1μF|Ceramic Capacitor|2012 SMD
C8|0.1μF|Ceramic Capacitor|2012 SMD
D1|RSX101VA-30TR|Shottky Barrier Diode|TUMD2
PGM|Green|LED|2012 SMD
PWR|Green|LED|2012 SMD
ERR|Red|LED|2012 SMD
HB|Yellow|LED|2012 SMD
RX|Yellow|LED|2012 SMD
TX|Yellow|LED|2012 SMD
IC1|74HC4053|ANALOG MULTIPLEXER|TSSOP-16
IC2|74HC4053|ANALOG MULTIPLEXER|TSSOP-16
L1|100μH|Inductor|3225 SMD
Q1|SI6544DQ|N- and P-Channel MOSFET|TSSOP-8
Q2|16MHz|Crystal|HS-49/S
R1|1Ω|Registor|2012 SMD
R2|1Ω|Registor|2012 SMD
R3|200Ω|Registor|2012 SMD
R4|1.1KΩ|Registor|2012 SMD
R5|100KΩ|Registor|2012 SMD
R6|130kΩ|Registor|2012 SMD
R7|3.3KΩ|Registor|2012 SMD
R8|10KΩ|Registor|2012 SMD
R9|15KΩ|Registor|2012 SMD
R10|10KΩ|Registor|2012 SMD
R11|1KΩ|Registor|2012 SMD
R12|1KΩ|Registor|2012 SMD
R13|1KΩ|Registor|2012 SMD
R14|1KΩ|Registor|2012 SMD
R15|1MΩ|Registor|2012 SMD
R16|1KΩ|Registor|2012 SMD
R17|1KΩ|Registor|2012 SMD
R18|1KΩ|Registor|2012 SMD
R19|10KΩ|Registor|2012 SMD
R20|10KΩ|Registor|2012 SMD
R21|10KΩ|Registor|2012 SMD
S1|SS-22SDP2|On-On Double Pole slide switch|NKK SS22SDP2
S2|RESET|Tactile Switch|
T1|2SC4116|NPN Transistor|SOT-23(SC-70)
T2|2SC4116|NPN Transistor|SOT-23(SC-70)
U2|MC34063ADG|Charge-pump DC-DC converter|SIOC-8
U3|IC-socket|28pin IC Socket|300MIL DIP
