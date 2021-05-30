#include "Arduino.h"
#include "daly-bms-uart.h"

//----------------------------------------------------------------------
// Public Functions
//----------------------------------------------------------------------

Daly_BMS_UART::Daly_BMS_UART(HardwareSerial &serial_peripheral)
{
    this->my_serialIntf = &serial_peripheral;
}

bool Daly_BMS_UART::Init()
{
    // Null check the serial interface
    if (this->my_serialIntf == NULL)
    {
#ifdef DALY_BMS_DEBUG
        Serial.println("<DALY-BMS DEBUG> ERROR: No serial peripheral specificed!");
#endif
        return false;
    }

    // Intialize the serial link to 9600 baud with 8 data bits and no parity bits, per the Daly BMS spec
    this->my_serialIntf->begin(9600, SERIAL_8N1);

    // Set up the output buffer with some values that won't be changing
    this->my_txBuffer[0] = 0xA5; // Start byte
    this->my_txBuffer[1] = 0x80; // Host address
    // this->my_txBuffer[2] is where our command ID goes
    this->my_txBuffer[3] = 0x08; // Length?

    // Fill bytes 5-11 with 0s
    for (uint8_t i = 4; i < 12; i++)
    {
        this->my_txBuffer[i] = 0x00;
    }

    return true;
}

bool Daly_BMS_UART::getPackMeasurements(float &voltage, float &current, float &SOC)
{
    this->sendCommand(COMMAND::VOUT_IOUT_SOC);

    if (!this->receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        Serial.printf("<DALY-BMS DEBUG> Receive failed, V, I, & SOC values won't be modified!\n");
#endif
        return false;
    }

    // Pull the relevent values out of the buffer
    voltage = (float)((this->my_rxBuffer[4] << 8) | this->my_rxBuffer[5]) / 10;
    // The current measurement is given with a 30000 unit offset
    current = (float)(((this->my_rxBuffer[8] << 8) | this->my_rxBuffer[9]) - 30000) / 10;
    SOC = (float)((this->my_rxBuffer[10] << 8) | this->my_rxBuffer[11]) / 10;

    return true;
}

bool Daly_BMS_UART::getPackTemp(int8_t &temp)
{
    this->sendCommand(COMMAND::MIN_MAX_TEMPERATURE);

    if (!this->receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        Serial.printf("<DALY-BMS DEBUG> Receive failed, Temp value won't be modified!\n");
#endif
        return false;
    }

    int8_t max_temp = (this->my_rxBuffer[4] - 40);
    int8_t min_temp = (this->my_rxBuffer[6] - 40);

    temp = (max_temp + min_temp) / 2;

    return true;
}

bool Daly_BMS_UART::getMinMaxCellVoltage(float &minCellV, uint8_t &minCellVNum, float &maxCellV, uint8_t &maxCellVNum)
{
    this->sendCommand(COMMAND::MIN_MAX_CELL_VOLTAGE);

    if (!receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        Serial.printf("<DALY-BMS DEBUG> Receive failed, min/max cell values won't be modified!\n");
#endif
        return false;
    }

    maxCellV = (float)((this->my_rxBuffer[4] << 8) | this->my_rxBuffer[5]) / 1000; // Given in mV, convert to V
    maxCellVNum = this->my_rxBuffer[6];
    minCellV = (float)((this->my_rxBuffer[7] << 8) | this->my_rxBuffer[8]) / 1000; // Given in mV, convert to V
    minCellVNum = this->my_rxBuffer[9];

    return true;
}

//----------------------------------------------------------------------
// Private Functions
//----------------------------------------------------------------------

void Daly_BMS_UART::sendCommand(COMMAND cmdID)
{
    this->my_txBuffer[2] = cmdID;

    // We can cheat a little when calculating the CRC of the outgoing UART transmission beacause
    // the only thing that changes in the outgoing buffer is the command, the rest stays the same.
    // Checksum = sum of all bytes, truncated to an 8 bit integer. See the readme for more info.
    // Checksum = (0xA5 + 0x80 + Command Num + 0x08) = (0x2D + Command Num)
    uint8_t checksum = 0x2D + cmdID;

    this->my_txBuffer[12] = checksum;

#ifdef DALY_BMS_DEBUG
    Serial.print("<DALY-BMS DEBUG> Checksum = 0x");
    Serial.println(checksum, HEX);
#endif

    this->my_serialIntf->write(this->my_txBuffer, XFER_BUFFER_LENGTH);
}

bool Daly_BMS_UART::receiveBytes(void)
{
    // Clear out the input buffer
    memset(this->my_rxBuffer, 0, XFER_BUFFER_LENGTH);

    // Read bytes from the specified serial interface
    uint8_t rxByteNum = this->my_serialIntf->readBytes(this->my_rxBuffer, XFER_BUFFER_LENGTH);

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
        checksum += this->my_rxBuffer[i];
    }

#ifdef DALY_BMS_DEBUG
    Serial.printf("<DALY-BMS DEBUG> Calculated checksum: 0x%x, Received checksum: 0x%x\n", checksum, this->my_rxBuffer[XFER_BUFFER_LENGTH - 1]);
#endif

    // Compare the calculated checksum to the real checksum (the last received byte)
    return (checksum == this->my_rxBuffer[XFER_BUFFER_LENGTH - 1]);
}

void Daly_BMS_UART::barfRXBuffer(void)
{
    Serial.printf("<DALY-BMS DEBUG> RX Buffer: [");
    for (int i = 0; i < XFER_BUFFER_LENGTH; i++)
    {
        Serial.printf("0x%x,", this->my_rxBuffer[i]);
    }
    Serial.printf("]\n");
}