/*
  MQGasSensor Library - reading an MQ4

  Demonstrates the use a MQ4 sensor.
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

 This example code is in the public domain.

*/

//Include the library
#include <MQGasSensor.h>

//Definitions
#define Voltage_Resolution 5
#define pin A4 //Analog input 4 of your arduino
#define type "MQ-4" //MQ4
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ4CleanAir 4.43 // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
//#define calibration_button 13 //Pin to calibrate your sensor

//Declare Sensor
MQGasSensor MQ4(pin, Voltage_Resolution, ADC_Bit_Resolution, type);

void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port

 
  
  
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
    MQ4.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ4.calibrate(RatioMQ4CleanAir);
    Serial.print(".");
  }
  MQ4.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println(F("Warning: R0 is infinite: the sensor output read 0 V. Check that the sensor is powered and AO is wired to the analog pin, not shorted to GND.")); while(1);}
  if(calcR0 == 0){Serial.println(F("Warning: R0 is zero: the sensor output read at or above the supply voltage. Check the supply and ADC reference settings, the divider on a 3.3 V board, and that AO is not shorted to VCC.")); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 

  /* 
    //If the RL value is different from 10K please set it with setLoadResistance:
    MQ4.setLoadResistance(10);
  */

  /*****************************  MQ Init ********************************************/ 
  //Input: setup flag, if this function are on setup will print the headers (Optional - Default value: False)
  //Output: print on serial port the information about sensor and sensor readings
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ4.begin(); 

  Serial.println("** Values from MQ-4**********");
  Serial.println("|      LPG (Linear EQ)        |         LPG (Exponential EQ)      |");  
}

void loop() {
  MQ4.update(); // Update data, the arduino will read the voltage from the analog pin
  
  //https://jayconsystems.com/blog/understanding-a-gas-sensor 
  //Set math model to calculate the PPM concentration and the value of constants
  MQ4.setCurve(-0.318, 1.133, MQModel::LogLinear); // ppm = 10^((log10(ratio) - b) / a); a is the slope, b the intercept
  float LPG1 = MQ4.readPpm(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  
  //Set math model to calculate the PPM concentration and the value of constants
  MQ4.setCurve(990.22, -2.826); // Configure the equation to to calculate CH4 concentration
  float LPG2 = MQ4.readPpm(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  //  exposure to 2000 ppm of LPG gas is immediately dangerous to life and health. In this section
  if(LPG1>=2000 || LPG2>=2000) Serial.println("Warning - Very high concentrations detected!");

  Serial.print("|              "); Serial.print(LPG1);
  Serial.print("            |           "); Serial.print(LPG2);
  Serial.println("          |");

  delay(500); //Sampling frequency
}