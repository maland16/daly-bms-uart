

#ifndef DALY_BMS_UART_H
#define DALY_BMS_UART_H

#define XFER_BUFFER_LENGTH 13

class Daly_BMS_UART
{
    public:
        enum COMMAND
        {
            VOUT_IOUT_SOC = 0x90,
            MIN_MAX_CELL_VOLTAGE = 0x91
        };

        Daly_BMS_UART(HardwareSerial & serialIntf);
        bool Init();
        uint16_t sendCommand(COMMAND cmdID);

    private:
        HardwareSerial * serialIntf;

        bool validateChecksum();

        uint8_t my_txBuffer[XFER_BUFFER_LENGTH];
        uint8_t my_rxBuffer[XFER_BUFFER_LENGTH];
};

#endif