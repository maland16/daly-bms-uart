# Arduino Library for basic communication with a DALY BMS
This library uses the Arduino Serial library to communicate with a DALY BMS over UART. It was originally designed for use with the **Teensy 4.0** as a part of [this project](https://github.com/maland16/citicar-charger) and has not been tested on official Arduino hardware. 

## How to use this library
-Beep  
-Bop  
-Boop  

## The DALY BMS UART Protocol
I found the UART Protocol used by the Daly BMS described in the PDF inside /docs/ on [diysolarform.com.](https://diysolarforum.com/resources/daly-smart-bms-manual-and-documentation.48/) It can be a little tough to decipher, so here's a brief overview.

Here's what an outgoing packet will look like. It's always fixed 13 bytes, and the reference manual from Daly doesn't mention anything about how to write data so the "Data" section of outgoing packsts is just always going to be 0. See "Future Improvements" below for more on this.
| Start Byte      | Host Address | Command ID | Data Length | Data | Checksum | 
| - | - | - | - | - | - | 
| 0xA5 | 0x80 | See below | 0x08 (fixed) | 0x0000000000000000 (8 bytes) | See below |

## Future Improvements
### The ability to write data to the BMS
The protocol description (see /docs/) doesn't mention anything about how to write data to the BMS, but it must be possible because the PC application (see /pc-software/) can set the parameters of the BMS. I've included some logic analyzer captures of communication between the BMS and PC application that someone can probably use to reverse engineer the protocol. I'm certain it's pretty simple, I honestly wouldn't be surprised if it were just the reading protocol with some small tweak.