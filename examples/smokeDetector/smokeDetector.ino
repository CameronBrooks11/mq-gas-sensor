/*
  MQGasSensor Library - smokeDetector
  
  Demonstrates the use a MQ2 sensor.
  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez
 
  Added example
  modified 23 May 2019
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
#define pin A4 //Analog input 4 of your arduino
#define type "MQ-4" //MQ4
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ4CleanAir 4.43 // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
//#define calibration_button 13 //Pin to calibrate your sensor

//Declare Sensor
MQGasSensor MQ4(pin, Voltage_Resolution, ADC_Bit_Resolution, type);

void setup() {
  //Init serial port
  Serial.begin(115200);
  //Set math model to calculate the PPM concentration and the value of constants
  MQ4.setCurve(4.7509e+07, -8.836); // Configure the equation to to calculate smoke concentration
  /*
    Exponential regression:
  Gas     | a          | b
  alcohol | 4.7360e+10 | -13.79
  CH4     | 990.22     | -2.826
  CO      | 3.6362e+14 | -19.54
  H2      | 360990     | -5.634
  LPG     | 3736.2     | -3.121
  smoke   | 4.7509e+07 | -8.836
  From docs/coefficients.md, the fits to the mq-4-hanwei-2007 datasheet.
  */
  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ4.begin(); 
   /* 
    //If the RL value is different from 10K please set it with setLoadResistance:
    MQ4.setLoadResistance(10);
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
    MQ4.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ4.calibrate(RatioMQ4CleanAir);
    Serial.print(".");
  }
  MQ4.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Connection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Connection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
}

  void loop() {
  /*****************************  MQReadSensor  ****************************************
  Input:   Gas - Serial print flag
  Output:  Value in PPM
  Remarks: This function use readPPM to read the value in PPM the gas in the air.
  ************************************************************************************/ 
  //Read the sensor and print in serial port
  //Lecture will be saved in lecture variable
  MQ4.update();
  float smokePPM = MQ4.readPpm(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  if(smokePPM > 1000) {Serial.println("Warning: High concentrations of smoke detected");}
  MQ4.printDebug(); // Print one line: voltage, Rs, R0, ratio and ppm
  delay(400);  
}
