#include <daly-bms-uart.h> // This is where the library gets pulled in

// Constructing the bms driver and passing in the Serial interface (which pins to use)
Daly_BMS_UART bms(Serial1);

void setup() {
  // This is needed to print stuff to the serial monitor
  Serial.begin(115200);

  // This call sets up the driver
  bms.Init();
}

void loop() {

  // Set up some variables to hold the values
  float volts = 0;
  float amps = 0;
  float percentage = 0;

  // Grab those values from the BMS
  bms.getPackMeasurements(volts, amps, percentage);

  // And print them out!
  Serial.printf("V: %4.1f, I: %4.1f, \%:%4.1f\n",volts, amps, percentage);

  // Now the same thing, but for temperature
  int8_t temp = 0;
  bms.getPackTemp(temp);
  Serial.printf("Temp: %d\n",temp);

  // And again, for min/max cell voltages
  float maxCellVoltage = 0;
  float minCellVoltage = 0;
  uint8_t maxCellNumber = 0;
  uint8_t minCellNumber = 0;
  bms.getMinMaxCellVoltage(minCellVoltage, minCellNumber, maxCellVoltage, maxCellNumber);
  Serial.printf("Highest Cell Voltage: Cell #%d with voltage %4.3f\n",maxCellNumber,maxCellVoltage);
  Serial.printf("Lowest Cell Voltage: Cell #%d with voltage %4.3f\n",minCellNumber,minCellVoltage);

  // Lets slow things down a bit...
  delay(500);
}
