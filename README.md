# Overview
This library uses the Arduino Serial library to communicate with a DALY BMS over UART. It was originally designed for use with the **Teensy 4.0** as a part of [this project](https://github.com/maland16/citicar-charger) and has not been tested on official Arduino hardware. 

## How to use this library  
-Download a zip of this library using the green button above  
-Follow [the instructions here](https://www.arduino.cc/en/guide/libraries) under "Manual installation"  
-Use the public functions defined in "daly-bms-uart.h" to your heart's content  
-Don't forget to construct a Daly_BMS_UART object and Init()!  
-See the example that's included in this library  

## Hardware setup
Below is a picture of the side of my DALY bms (yours might look slightly different) showing which pins are used to communicate over UART. 
<img src="https://raw.githubusercontent.com/maland16/daly-bms-uart/main/docs/UART%20Interface.jpg">
I used this library on a teensy 4.0, who's serial pins are only 3V tolerant, so I also used [this logic level shifter](https://www.adafruit.com/product/757) to bring the logic level up to 5V, which is what I observed while using the PC UART USB adapter included with the BMS.

## The DALY BMS UART Protocol
I found the UART Protocol used by the Daly BMS described in the PDF inside /docs/ on [diysolarform.com.](https://diysolarforum.com/resources/daly-smart-bms-manual-and-documentation.48/) It can be a little tough to decipher, so here's a brief overview.

Here's what an outgoing packet will look like. It's always fixed 13 bytes, and the reference manual from Daly doesn't mention anything about how to write data so the "Data" section of outgoing packets is just always going to be 0. See "Future Improvements" below for more on this.
| Start Byte      | Host Address | Command ID | Data Length | Data | Checksum | 
| - | - | - | - | - | - | 
| 0xA5 | 0x80 | See below | 0x08 (fixed) | 0x0000000000000000 (8 bytes) | See below |

This is what an incoming packet might look like. In this case it's the "Voltage, Current, and SOC" command. 
| Start Byte      | Host Address | Command ID | Data Length | Data | Checksum | 
| - | - | - | - | - | - | 
| 0xA5 | 0x01 | 0x90 (see below) | 0x08 (fixed?*) | 0x023A0000753001ED (8 bytes) | 0x0D (See below) |

\*It's not made totally clear in the protocol description but it seems like the received data length might actually be longer for certain commands. Reading all cell voltages & all temperature sensor readings are examples of commands that could have much longer data sections.  

#### Data section
The first two bytes of the Data correspond to the Voltage in tenths of volts (0x023A = 570 = 57.0V). I'm honestly not sure what the next two bytes are for, the documentation calls them "acquisition voltage". They always come back 0 for me so lets skip them. The next two bytes are the current in tenths of amps, with an offset of 30000 (0x7530 = 300000 - 30,000 = 0 = 0.0A). The final two bytes are the state of chare (or SOC) in tenths of a percent (0x01ED = 493 = 49.3%).   
#### Checksum
The last byte of the packet is a checksum, which is calculated by summing up all the rest of the bytes in the packet and truncating the result to one byte. (0xA5 + 0x01 + 0x90 + ... + 0xED = 0x30D = 0x0D).  

### Supported Commands
Here's an overview of the commands that are supported by this library. See the full protocol info in /docs/ for more info.  
| Command | Hex | Support API |  
| - | - | - |
| Voltage, Current, SOC | 0x90 | getPackMeasurements() |  
| Min & Max Cell Voltages | 0x91 | getMinMaxCellVoltage() |  
| Min & Max Temp Sensor readings | 0x92 | getPackTemp() will take the min and max temperature readings, average them, and return that value. Most of the DALY BMSs that I've seen only have one temperature sensor. |  

## Troubleshooting
- The BMS has no internal power source, and needs to be connected to the battery for the UART communication to work.
- Make sure your Tx/Rx aren't mixed up, in the picture above Tx/Rx are labeled with respect to the BMS.  
- I could not have made this work/debugged this without a logic analyzer hooked up to the UART lines to see what's going on. They can be had pretty cheaply and are an invaluable tool for working on these kinds of things.  
- Uncomment [this define](https://github.com/maland16/daly-bms-uart/blob/main/daly-bms-uart.h#L8) to get lots of debug printing to the Arduino Serial Monitor. I added these statements to help as I developed the code and ran into issues. Beyond that, I've done my best to comment extensively.  

## Future Improvements
### The ability to write data to the BMS
The protocol description (see /docs/) doesn't mention anything about how to write data to the BMS, but it must be possible because the PC application (see /pc-software/) can set the parameters of the BMS. I've included some logic analyzer captures of communication between the BMS and PC application that someone can probably use to reverse engineer the protocol. I'm certain it's pretty simple, I honestly wouldn't be surprised if it were just the reading protocol with some small tweak.
### Support for more commands
There are a bunch of additional commands that are outlined in the protocol description document, so far I've only added support for the ones I intend to use.
### Redesign
This library was purpose built for [this project](https://github.com/maland16/citicar-charger) and I made it a library in the hopes someone would get some utility out of it, but it could use to be redesigned from the ground up to be more capable/useful.