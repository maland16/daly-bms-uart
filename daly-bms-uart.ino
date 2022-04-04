#include <Arduino.h>

#include <daly-bms-uart.h> // This is where the library gets pulled in

#define BMS_SERIAL Serial  //Set the serial port for communication with the Daly BMS


// NOTE: You must define and call DEBUG_SERIAL.begin(<baud rate>) in your setup() for this to work
#define DEBUG_SERIAL Serial1 //Set the Serial Debug port
#define DALY_BMS_DEBUG // Uncomment the below #define to enable debugging print statements.


// Constructing the bms driver and passing in the Serial interface (which pins to use)
Daly_BMS_UART bms(BMS_SERIAL);
void setup()
{
  // This is needed to print stuff to the serial monitor
  DEBUG_SERIAL.begin(9600);

  // This call sets up the driver
  bms.Init();
}

void loop()
{

  // call to update the data from the bms
  bms.update();

  // And print them out!
  DEBUG_SERIAL.println("basic BMS Data:              " + (String)bms.get.packVoltage + "V " + (String)bms.get.packCurrent + "I " + (String)bms.get.packSOC + "\% ");
  DEBUG_SERIAL.println("Package Temperature:         " + (String)bms.get.tempAverage);
  DEBUG_SERIAL.println("Highest Cell Voltage Nr:     " + (String)bms.get.maxCellVNum + " with voltage " + (String)(bms.get.maxCellmV / 1000));
  DEBUG_SERIAL.println("Lowest Cell Voltage Nr:      " + (String)bms.get.minCellVNum + " with voltage " + (String)(bms.get.minCellmV / 1000));
  DEBUG_SERIAL.println("Number of Cells:             " + (String)bms.get.numberOfCells);
  DEBUG_SERIAL.println("Number of Temp Sensors:      " + (String)bms.get.numOfTempSensors);
  DEBUG_SERIAL.println("BMS Chrg / Dischrg Cycles:   " + (String)bms.get.bmsCycles);
  DEBUG_SERIAL.println("BMS HEartbeat:               " + (String)bms.get.bmsHeartBeat); //cycle 0-255
  DEBUG_SERIAL.println("Discharge MOSFet Status:     " + (String)bms.get.disChargeFetState);
  DEBUG_SERIAL.println("Charge MOSFet Status:        " + (String)bms.get.chargeFetState);
  DEBUG_SERIAL.println("Remaining Capacity mAh:      " + (String)bms.get.resCapacitymAh);

  //for alarm flags - for all flags see the alarm struct in daly-bms-uart.h and refer to the datasheet
  DEBUG_SERIAL.println("Level one Cell V to High:    " + (String)bms.alarm.levelOneCellVoltageTooHigh);

/**
 * Advanced functions:
 * bms.setBmsReset(); //Reseting the BMS, after reboot the MOS Gates are enabled!
 * bms.setDischargeMOS(true); Switches on the discharge Gate
 * bms.setDischargeMOS(false); Switches off thedischarge Gate
 * bms.setChargeMOS(true); Switches on the charge Gate
 * bms.setChargeMOS(false); Switches off the charge Gate
 */
 
}
