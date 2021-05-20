#include "Arduino.h"
#include "daly-bms-uart.h"

Daly_BMS_UART::Daly_BMS_UART(HardwareSerial & serial_peripheral)
{
    serialIntf = &serial_peripheral;
}

bool Daly_BMS_UART::Init()
{
    // Null check the serial interface
    if(serialIntf == NULL)
    {
        Serial.println("ERROR: No serial peripheral specificed!");
        return false;
    }

    // Intialize the serial link to 9600 baud with 8 data bits and no parity bits, per the Daly BMS spec
    serialIntf->begin(9600, SERIAL_8N1);

    // Set up the output buffer with some values that won't be changing
    my_txBuffer[0] = 0xA5; // Start byte
    my_txBuffer[1] = 0x80; // Host address
    my_txBuffer[3] = 0x08; // Length?
    // my_txBuffer[4] is where our command ID goes

    // Fill bytes 5-11 with 0s
    for(uint8_t i = 5; i < 12; i++)
    {
        my_txBuffer[i] = 0x00;
    }

    return true;
}



uint16_t Daly_BMS_UART::sendCommand(COMMAND cmdID)
{
    my_txBuffer[2] = cmdID;
    
    // 0x2D = 0xA5 + 0x80 + 0x08
    uint8_t checksum = 0x2D + cmdID;
    
    my_txBuffer[12] = checksum;

    Serial.print("DEBUG: Checksum = 0x");
    Serial.println(checksum,HEX);

    serialIntf->write(my_txBuffer, XFER_BUFFER_LENGTH);

    // Wait a lil for the BMS to respond (generally takes ~20 ms)
    delay(30);

    // Clear out the input buffer
    memset(my_rxBuffer, 0, XFER_BUFFER_LENGTH);
    
    uint8_t rxByteNum = 0;

    // Grab the incoming data from the teensy's internal serial buffer
    while(serialIntf->available() > 0)
    {
        my_rxBuffer[rxByteNum] = serialIntf->read();

        rxByteNum++;
    }

    // Make sure we got the correct number of bytes
    if(rxByteNum != (XFER_BUFFER_LENGTH - 1))
    {
        Serial.print("ERROR: Recieved the wrong number of bytes! Expected 13, got ");
        Serial.println(rxByteNum,DEC);
        return 0x00;
    }

    if(!validateChecksum())
    {
        Serial.println("ERROR: Checksum failed!");
        return 0x00;
    }

    uint16_t pressure = my_rxBuffer[]
}

/**
 * @brief Validates the checksum in the RX Buffer
 * @return true if checksum passes, false otherwise
 */
bool Daly_BMS_UART::validateChecksum()
{
    uint8_t checksum = 0x00;

    for(int i = 0; i < XFER_BUFFER_LENGTH - 1; i++)
    {
        checksum += my_rxBuffer[i];
    }

    return (checksum == my_rxBuffer[XFER_BUFFER_LENGTH]);
}