#include "Arduino.h"
#include "daly-bms-uart.h"

Daly_BMS_UART::Daly_BMS_UART(HardwareSerial &serial_peripheral)
{
    serialIntf = &serial_peripheral;
}

bool Daly_BMS_UART::Init()
{
    // Null check the serial interface
    if (serialIntf == NULL)
    {
        Serial.println("<DALY-BMS DEBUG> ERROR: No serial peripheral specificed!");
        return false;
    }

    // Intialize the serial link to 9600 baud with 8 data bits and no parity bits, per the Daly BMS spec
    serialIntf->begin(9600, SERIAL_8N1);

    // Set up the output buffer with some values that won't be changing
    my_txBuffer[0] = 0xA5; // Start byte
    my_txBuffer[1] = 0x80; // Host address
    // my_txBuffer[2] is where our command ID goes
    my_txBuffer[3] = 0x08; // Length?

    // Fill bytes 5-11 with 0s
    for (uint8_t i = 4; i < 12; i++)
    {
        my_txBuffer[i] = 0x00;
    }

    return true;
}

bool Daly_BMS_UART::getPackMeasurements(uint16_t &voltage, int16_t &current, uint16_t &SOC)
{
    sendCommand(COMMAND::VOUT_IOUT_SOC);

    // Wait a small bit for the BMS to respond (generally takes ~20 ms)
    delay(30);

    receiveBytes();
    //if (!receiveBytes())
    //{
    //    return false;
    //}

    // Pull the relevent values out of the buffer
    voltage = (my_rxBuffer[4] << 8) | my_rxBuffer[5];
    current = (my_rxBuffer[8] << 8) | my_rxBuffer[9];
    SOC = (my_rxBuffer[10] << 8) | my_rxBuffer[11];

    return true;
}

bool Daly_BMS_UART::getPackTemp(int8_t &temp)
{
}

void Daly_BMS_UART::sendCommand(COMMAND cmdID)
{
    my_txBuffer[2] = cmdID;

    // We can cheat a little when calculating the CRC of the outgoing UART transmission beacause
    // the only thing that changes in the outgoing buffer is the command, the rest stays the same.
    // Checksum = sum of all bytes, truncated to an 8 bit integer. See the readme for more info.
    // Checksum = 0xA5 + 0x80 + Command Num + 0x08 = 0x2D + Command Num
    uint8_t checksum = 0x2D + cmdID;

    my_txBuffer[12] = checksum;

    Serial.print("<DALY-BMS DEBUG> Checksum = 0x");
    Serial.println(checksum, HEX);

    serialIntf->write(my_txBuffer, XFER_BUFFER_LENGTH);
}

bool Daly_BMS_UART::receiveBytes(void)
{
    // Clear out the input buffer
    memset(my_rxBuffer, 0, XFER_BUFFER_LENGTH);

    uint8_t rxByteNum = 0;

    // Grab the incoming data from the teensy's internal serial buffer
    while (serialIntf->available() > 0)
    {
        my_rxBuffer[rxByteNum] = serialIntf->read();

        rxByteNum++;
    }

    // Make sure we got the correct number of bytes
    if (rxByteNum != (XFER_BUFFER_LENGTH - 1))
    {
        Serial.print("<DALY-BMS DEBUG> Error: Received the wrong number of bytes! Expected 13, got ");
        Serial.println(rxByteNum, DEC);
        return false;
    }

    if (!validateChecksum())
    {
        Serial.println("<DALY-BMS DEBUG> Error: Checksum failed!");
        return false;
    }

    return true;
}

bool Daly_BMS_UART::validateChecksum()
{
    uint8_t checksum = 0x00;

    for (int i = 0; i < XFER_BUFFER_LENGTH - 1; i++)
    {
        checksum += my_rxBuffer[i];
    }

    return (checksum == my_rxBuffer[XFER_BUFFER_LENGTH]);
}