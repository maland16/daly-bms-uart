#include "Arduino.h"
#include "daly-bms-uart.h"
#define DALY_BMS_DEBUG
#define DEBUG_SERIAL Serial

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
        DEBUG_SERIAL.println("<DALY-BMS DEBUG> ERROR: No serial peripheral specificed!");
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

bool Daly_BMS_UART::update()
{

    getPackMeasurements();
    getMinMaxCellVoltage();
    getPackTemp();
    getStatusInfo();
    //getCellVoltages(); //dont work, the answer string from bms is too long and must splitet, i have no idea
   //getFailureCodes(); //coming soon
/**
 * put here the function call to recive all data one by one
 * check if cell number ar set and then ask for cell data
 * 
 */

    if (!this->receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        DEBUG_SERIAL.print("Updating the Data Failed\n");
#endif
        return false;
    }
    return true;
}


bool Daly_BMS_UART::getPackMeasurements()
{
    this->sendCommand(COMMAND::VOUT_IOUT_SOC);

    if (!this->receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        DEBUG_SERIAL.print("<DALY-BMS DEBUG> Receive failed, V, I, & SOC values won't be modified!\n");
#endif
        return false;
    }
    // Pull the relevent values out of the buffer
    get.packVoltage = (float)((this->my_rxBuffer[4] << 8) | this->my_rxBuffer[5]) / 10;
    // The current measurement is given with a 30000 unit offset
    get.packCurrent = (float)(((this->my_rxBuffer[8] << 8) | this->my_rxBuffer[9]) - 30000) / 10;
    get.packSOC = (float)((this->my_rxBuffer[10] << 8) | this->my_rxBuffer[11]) / 10;
    return true;
}

bool Daly_BMS_UART::getPackTemp()
{
    this->sendCommand(COMMAND::MIN_MAX_TEMPERATURE);

    if (!this->receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        DEBUG_SERIAL.print("<DALY-BMS DEBUG> Receive failed, Temp value won't be modified!\n");
#endif
        return false;
    }

    uint8_t max_temp = (this->my_rxBuffer[4] - 40); //byte 0 from datasheet
    uint8_t min_temp = (this->my_rxBuffer[6] - 40); //byte 3 from datasheet
    get.tempAverage = (max_temp + min_temp) / 2;

    return true;
}

bool Daly_BMS_UART::getMinMaxCellVoltage()
{
    this->sendCommand(COMMAND::MIN_MAX_CELL_VOLTAGE);

    if (!receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        DEBUG_SERIAL.print("<DALY-BMS DEBUG> Receive failed, min/max cell values won't be modified!\n");
#endif
        return false;
    }

    get.maxCellmV = (float)((this->my_rxBuffer[4] << 8) | this->my_rxBuffer[5]); // Given in mV, convert to V
    get.maxCellVNum = this->my_rxBuffer[6];
    get.minCellmV = (float)((this->my_rxBuffer[7] << 8) | this->my_rxBuffer[8]); // Given in mV, convert to V
    get.minCellVNum = this->my_rxBuffer[9];

    return true;
}
bool Daly_BMS_UART::getStatusInfo()
{
    this->sendCommand(COMMAND::STATUS_INFO);

    if (!receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        DEBUG_SERIAL.print("<DALY-BMS DEBUG> Receive failed, Status info won't be modified!\n");
#endif
        return false;
    }
    get.numberOfCells =     this->my_rxBuffer[4];
    get.numOfTempSensors =  this->my_rxBuffer[5];
    get.dischargeState =    this->my_rxBuffer[6];
    get.chargeState =       this->my_rxBuffer[7];

    for (size_t i = 0; i < 8; i++)
    {
        get.dIO[i] = bitRead(this->my_rxBuffer[8],i);
    }
    get.bmsCycles = this->my_rxBuffer[9];
    
    return true;
}

bool Daly_BMS_UART::getCellVoltages()
{
    this->sendCommand(COMMAND::CELL_VOLTAGES);

    if (!receiveBytes())
    {
#ifdef DALY_BMS_DEBUG
        DEBUG_SERIAL.print("<DALY-BMS DEBUG> Receive failed, Cell Voltages won't be modified!\n");
#endif
        return false;
    }
    for (size_t i = 4; i < 9; i++)
    {
        DEBUG_SERIAL.println(this->my_rxBuffer[i]);
    }
    
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
    DEBUG_SERIAL.print("<DALY-BMS DEBUG> Checksum = 0x");
    DEBUG_SERIAL.println(checksum, HEX);
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
        DEBUG_SERIAL.print("<DALY-BMS DEBUG> Error: Received the wrong number of bytes! Expected 13, got ");
        DEBUG_SERIAL.println(rxByteNum, DEC);
        this->barfRXBuffer();
#endif
        return false;
    }

    if (!validateChecksum())
    {
#ifdef DALY_BMS_DEBUG
        DEBUG_SERIAL.println("<DALY-BMS DEBUG> Error: Checksum failed!");
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
    DEBUG_SERIAL.print("<DALY-BMS DEBUG> Calculated checksum: 0x"+(String)checksum+", Received checksum: 0x"+(String)this->my_rxBuffer[XFER_BUFFER_LENGTH - 1]+"\n");
#endif

    // Compare the calculated checksum to the real checksum (the last received byte)
    return (checksum == this->my_rxBuffer[XFER_BUFFER_LENGTH - 1]);
}

void Daly_BMS_UART::barfRXBuffer(void)
{
    DEBUG_SERIAL.print("<DALY-BMS DEBUG> RX Buffer: [");
    for (int i = 0; i < XFER_BUFFER_LENGTH; i++)
    {
        DEBUG_SERIAL.print("0x"+(String)this->my_rxBuffer[i]);
    }
    DEBUG_SERIAL.print("]\n");
}
