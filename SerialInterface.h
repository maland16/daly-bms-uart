#include "SoftwareSerial.h"
#include "Arduino.h"

#ifndef DALY_BMS_UART_MAIN_SOFTSERIAL_ISERIALINTERFACE_H
#define DALY_BMS_UART_MAIN_SOFTSERIAL_ISERIALINTERFACE_H

class SerialInterface {
public:
    virtual void begin(int baud) = 0;
    virtual int read() = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
    virtual size_t println(String value) = 0;
    virtual size_t readBytes(uint8_t *buffer, size_t length) = 0;
};

class HardwareSerialInterface: public SerialInterface {
public:
    HardwareSerialInterface(HardwareSerial *hwSerial);

    void begin(int baud) override;
    int read() override;
    size_t write(const uint8_t *buffer, size_t size) override;
    size_t println(String value) override;
    size_t readBytes(uint8_t *buffer, size_t length) override;
private:
    HardwareSerial *HwSerial;
};

class SoftwareSerialInterface: public SerialInterface {
public:
    SoftwareSerialInterface(SoftwareSerial *softwareSerial);

    void begin(int baud) override;
    int read() override;
    size_t write(const uint8_t *buffer, size_t size) override;
    size_t println(String value) override;
    size_t readBytes(uint8_t *buffer, size_t length) override;
private:
    SoftwareSerial *SwSerial;
};


#endif //DALY_BMS_UART_MAIN_SOFTSERIAL_ISERIALINTERFACE_H
