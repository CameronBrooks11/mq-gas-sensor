/*
  MQGasSensor Library - reading an MQ303

  Demonstrates the use a MQ303 sensor.
  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez
 
  Added example
  modified 23 May 2019
  by Miguel Califa 

  Updated library usage
  modified 26 March 2020
  by Miguel Califa 

  Wiring:
  https://github.com/CameronBrooks11/mq-gas-sensor/blob/main/docs/img/MQ_Arduino.PNG
  Please make sure arduino A0 pin represents the analog input configured on #define pin

 This example code is in the public domain.

*/

//Include the library
#include <MQGasSensor.h>

//Definitions
#define Voltage_Resolution 5
#define pin A0 //Analog input 0 of your arduino
#define type "MQ-303A" //MQ303A
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ303CleanAir 1 // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
//#define calibration_button 13 //Pin to calibrate your sensor

//Declare Sensor
MQGasSensor MQ303(pin, Voltage_Resolution, ADC_Bit_Resolution, type);

void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port

  //Set math model to calculate the PPM concentration and the value of constants
  MQ303.setCurve(5.3977, -3.001); // Configure the equation to to calculate iso-butane concentration
  MQ303.setSupplyOffset(0.45); // The MQ-303A's Rs is computed from the supply less 0.45 V

  /*
    Exponential regression:
  Gas        | a          | b
  ethanol    | 3.5821     | -2.423
  H2         | 4.3296     | -2.721
  iso-butane | 5.3977     | -3.001
  From docs/coefficients.md, the fits to the mq-303a-hanwei-2006 datasheet.
  */

  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ303.begin(); 
  /* 
    //If the RL value is different from 10K please set it with setLoadResistance:
    MQ303.setLoadResistance(10);
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
   // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
  // and on clean air (Calibration conditions), setting up R0 value.
  // We recommend executing this routine only on setup in laboratory conditions.
  // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ303.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ303.calibrate(RatioMQ303CleanAir);
    Serial.print(".");
  }
  MQ303.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println(F("Warning: R0 is infinite: the sensor output read 0 V. Check that the sensor is powered and AO is wired to the analog pin, not shorted to GND.")); while(1);}
  if(calcR0 == 0){Serial.println(F("Warning: R0 is zero: the sensor output read at or above the supply voltage. Check the supply and ADC reference settings, the divider on a 3.3 V board, and that AO is not shorted to VCC.")); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
}

void loop() {
  MQ303.update(); // Update data, the arduino will read the voltage from the analog pin
  MQ303.readPpm(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  MQ303.printDebug(); // Print one line: voltage, Rs, R0, ratio and ppm
  delay(500); //Sampling frequency
}