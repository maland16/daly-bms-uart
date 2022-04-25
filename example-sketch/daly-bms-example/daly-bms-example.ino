#include <Arduino.h>

#include <daly-bms-uart.h> // This is where the library gets pulled in

#define BMS_SERIAL Serial1 // Set the serial port for communication with the Daly BMS
                           // Set the Serial Debug port

// To print debug info from the inner workings of the library, see the README

// Construct the BMS driver and passing in the Serial interface (which pins to use)
Daly_BMS_UART bms(BMS_SERIAL);

void setup()
{
  // Used for debug printing
  Serial.begin(9600); // Serial interface for the Arduino Serial Monitor

  bms.Init(); // This call sets up the driver
}

void loop()
{
  // Print a message and wait for input from the user
  Serial.println("Press any key to query data from the BMS");
  while (Serial.available() == 0)
  {
  }
  Serial.read();
  Serial.read(); // To discard the new line that gets sent

  // call to update the data from the bms
  bms.update();

  // And print them out!
  Serial.println("basic BMS Data:              " + (String)bms.get.packVoltage + "V " + (String)bms.get.packCurrent + "I " + (String)bms.get.packSOC + "\% ");
  Serial.println("Package Temperature:         " + (String)bms.get.tempAverage);
  Serial.println("Highest Cell Voltage Nr:     " + (String)bms.get.maxCellVNum + " with voltage " + (String)(bms.get.maxCellmV / 1000));
  Serial.println("Lowest Cell Voltage Nr:      " + (String)bms.get.minCellVNum + " with voltage " + (String)(bms.get.minCellmV / 1000));
  Serial.println("Number of Cells:             " + (String)bms.get.numberOfCells);
  Serial.println("Number of Temp Sensors:      " + (String)bms.get.numOfTempSensors);
  Serial.println("BMS Chrg / Dischrg Cycles:   " + (String)bms.get.bmsCycles);
  Serial.println("BMS Heartbeat:               " + (String)bms.get.bmsHeartBeat); // cycle 0-255
  Serial.println("Discharge MOSFet Status:     " + (String)bms.get.disChargeFetState);
  Serial.println("Charge MOSFet Status:        " + (String)bms.get.chargeFetState);
  Serial.println("Remaining Capacity mAh:      " + (String)bms.get.resCapacitymAh);
  //... any many many more data

  for (size_t i = 0; i < size_t(bms.get.numberOfCells); i++)
  {
    Serial.println("Remaining Capacity mAh:      " + (String)bms.get.cellVmV[i]);
  }

  // for alarm flags - for all flags see the alarm struct in daly-bms-uart.h and refer to the datasheet
  Serial.println("Level one Cell V to High:    " + (String)bms.alarm.levelOneCellVoltageTooHigh);

  /**
   * Advanced functions:
   * bms.setBmsReset(); //Reseting the BMS, after reboot the MOS Gates are enabled!
   * bms.setDischargeMOS(true); Switches on the discharge Gate
   * bms.setDischargeMOS(false); Switches off thedischarge Gate
   * bms.setChargeMOS(true); Switches on the charge Gate
   * bms.setChargeMOS(false); Switches off the charge Gate
   */
}

/*
#include <daly-bms-uart.h> // This is where the library gets pulled in

// Constructing the bms driver and passing in the Serial interface (which pins to use)
Daly_BMS_UART bms(Serial1);

void setup()
{
  // This is needed to print stuff to the serial monitor
  Serial.begin(115200);

  // This call sets up the driver
  bms.Init();
}

void loop()
{

  // Set up some variables to hold the values
  float volts = 0;
  float amps = 0;
  float percentage = 0;

  // Grab those values from the BMS
  bms.getPackMeasurements(volts, amps, percentage);

  // And print them out!
  Serial.printf("V: %4.1f, I: %4.1f, SOC:%4.1f\n", volts, amps, percentage);

  // Now the same thing, but for temperature
  int8_t temp = 0;
  bms.getPackTemp(temp);
  Serial.printf("Temp: %d\n", temp);

  // And again, for min/max cell voltages
  float maxCellVoltage = 0;
  float minCellVoltage = 0;
  uint8_t maxCellNumber = 0;
  uint8_t minCellNumber = 0;
  bms.getMinMaxCellVoltage(minCellVoltage, minCellNumber, maxCellVoltage, maxCellNumber);
  Serial.printf("Highest Cell Voltage: Cell #%d with voltage %4.3f\n", maxCellNumber, maxCellVoltage);
  Serial.printf("Lowest Cell Voltage: Cell #%d with voltage %4.3f\n", minCellNumber, minCellVoltage);

  // Lets slow things down a bit...
  delay(500);
} */
