#include "SerialInterface.h"

HardwareSerialInterface::HardwareSerialInterface(HardwareSerial *hwSerial){
    HwSerial = hwSerial;
}

void HardwareSerialInterface::begin(int baud){
    HwSerial->begin(baud);
}

int HardwareSerialInterface::read(){
    return HwSerial->read();
}

size_t HardwareSerialInterface::write(const uint8_t *buffer, size_t size){
    return HwSerial->write(buffer, size);
}

size_t HardwareSerialInterface::println(String value){
    return HwSerial->println(value);
}

size_t HardwareSerialInterface::readBytes(uint8_t *buffer, size_t length){
    return HwSerial->readBytes(buffer, length);
}

SoftwareSerialInterface::SoftwareSerialInterface(SoftwareSerial *softwareSerial){
    SwSerial = softwareSerial;
}

void SoftwareSerialInterface::begin(int baud){
    SwSerial->begin(baud);
}

int SoftwareSerialInterface::read(){
    return SwSerial->read();
}

size_t SoftwareSerialInterface::write(const uint8_t *buffer, size_t size){
    return SwSerial->write(buffer, size);
}

size_t SoftwareSerialInterface::println(String value){
    return SwSerial->println(value);
}

size_t SoftwareSerialInterface::readBytes(uint8_t *buffer, size_t length){
    return SwSerial->readBytes(buffer, length);
}