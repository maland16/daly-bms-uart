#include <Arduino.h>
#include <daly-bms-uart.h> // This is where the library gets pulled in

// Constructing the bms driver and passing in the Serial interface (which pins to use)
Daly_BMS_UART bms(Serial1);
void setup() {
  // This is needed to print stuff to the serial monitor
  Serial.begin(9600);

  // This call sets up the driver
  bms.Init();
}

void loop() {

  // Grab those values from the BMS
  //bms.getPackMeasurements();

  bms.update();


  // And print them out!
  Serial.println((String)bms.get.packVoltage+"V "+(String)bms.get.packCurrent+"I "+(String)bms.get.packSOC+"\% ");

  // Now the same thing, but for temperature
  //bms.getPackTemp();
  Serial.println("Package Temperature: "+(String)bms.get.tempAverage);

  // And again, for min/max cell voltages
  //bms.getMinMaxCellVoltage();
  Serial.println("Highest Cell Voltage Nr:"+(String)bms.get.maxCellVNum+" with voltage "+(String)(bms.get.maxCellmV / 1000));
  Serial.println("Lowest Cell Voltage Nr:"+(String)bms.get.minCellVNum+" with voltage "+(String)(bms.get.minCellmV / 1000));
 

//Serial.print("\r");
  // Lets slow things down a bit...
  //delay(500);
}
