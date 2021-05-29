#include "Arduino.h"
#include "daly-bms-uart.h"

//----------------------------------------------------------------------
// Public Functions
//----------------------------------------------------------------------

Daly_BMS_UART::Daly_BMS_UART(HardwareSerial &serial_peripheral)
{
    serialIntf = &serial_peripheral;
}

bool Daly_BMS_UART::Init()
{
    // Null check the serial interface
    if (serialIntf == NULL)
    {
#ifdef DALY_BMS_DEBUG
        Serial.println("<DALY-BMS DEBUG> ERROR: No serial peripheral specificed!");
#endif
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

    if (!receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        Serial.printf("<DALY-BMS DEBUG> Receive failed, V, I, & SOC values won't be modified!\n");
#endif
        return false;
    }

    // Pull the relevent values out of the buffer
    voltage = (my_rxBuffer[4] << 8) | my_rxBuffer[5];
    // The current measurement is given with a 30000 unit offset
    current = ((my_rxBuffer[8] << 8) | my_rxBuffer[9]) - 30000;
    SOC = (my_rxBuffer[10] << 8) | my_rxBuffer[11];

    return true;
}

bool Daly_BMS_UART::getPackTemp(int8_t &temp)
{
    sendCommand(COMMAND::MIN_MAX_TEMPERATURE);

    if (!receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        Serial.printf("<DALY-BMS DEBUG> Receive failed, Temp value won't be modified!\n");
#endif
        return false;
    }

    int8_t max_temp = (my_rxBuffer[4] - 40);
    int8_t min_temp = (my_rxBuffer[6] - 40);

    temp = (max_temp + min_temp) / 2;

    return true;
}

bool Daly_BMS_UART::getMinMaxCellVoltage(float &minCellV, uint8_t &minCellVNum, float &maxCellV, uint8_t &maxCellVNum)
{
    sendCommand(COMMAND::MIN_MAX_CELL_VOLTAGE);

    if (!receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        Serial.printf("<DALY-BMS DEBUG> Receive failed, min/max cell values won't be modified!\n");
#endif
        return false;
    }

    maxCellV = (float)((my_rxBuffer[4] << 8) | my_rxBuffer[5]) / 1000; // Given in mV, convert to V
    maxCellVNum = my_rxBuffer[6];
    minCellV = (float)((my_rxBuffer[7] << 8) | my_rxBuffer[8]) / 1000; // Given in mV, convert to V
    minCellVNum = my_rxBuffer[9];
}

//----------------------------------------------------------------------
// Private Functions
//----------------------------------------------------------------------

void Daly_BMS_UART::sendCommand(COMMAND cmdID)
{
    my_txBuffer[2] = cmdID;

    // We can cheat a little when calculating the CRC of the outgoing UART transmission beacause
    // the only thing that changes in the outgoing buffer is the command, the rest stays the same.
    // Checksum = sum of all bytes, truncated to an 8 bit integer. See the readme for more info.
    // Checksum = 0xA5 + 0x80 + Command Num + 0x08 = 0x2D + Command Num
    uint8_t checksum = 0x2D + cmdID;

    my_txBuffer[12] = checksum;

#ifdef DALY_BMS_DEBUG
    Serial.print("<DALY-BMS DEBUG> Checksum = 0x");
    Serial.println(checksum, HEX);
#endif

    serialIntf->write(my_txBuffer, XFER_BUFFER_LENGTH);
}

bool Daly_BMS_UART::receiveBytes(void)
{
    // Clear out the input buffer
    memset(my_rxBuffer, 0, XFER_BUFFER_LENGTH);

    uint8_t rxByteNum = serialIntf->readBytes(my_rxBuffer, XFER_BUFFER_LENGTH);

    // Make sure we got the correct number of bytes
    if (rxByteNum != XFER_BUFFER_LENGTH)
    {
#ifdef DALY_BMS_DEBUG
        Serial.print("<DALY-BMS DEBUG> Error: Received the wrong number of bytes! Expected 13, got ");
        Serial.println(rxByteNum, DEC);
        this->barfRXBuffer();
#endif
        return false;
    }

    if (!validateChecksum())
    {
#ifdef DALY_BMS_DEBUG
        Serial.println("<DALY-BMS DEBUG> Error: Checksum failed!");
        this->barfRXBuffer();
#endif
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

#ifdef DALY_BMS_DEBUG
    Serial.printf("<DALY-BMS DEBUG> Calculated checksum: 0x%x, Received checksum: 0x%x\n", checksum, my_rxBuffer[XFER_BUFFER_LENGTH - 1]);
#endif

    // Compare the calculated checksum to the real checksum (the last received byte)
    return (checksum == my_rxBuffer[XFER_BUFFER_LENGTH - 1]);
}

void Daly_BMS_UART::barfRXBuffer(void)
{
    Serial.printf("<DALY-BMS DEBUG> RX Buffer: [");
    for (int i = 0; i < XFER_BUFFER_LENGTH; i++)
    {
        Serial.printf("0x%x,", my_rxBuffer[i]);
    }
    Serial.printf("]\n");
}