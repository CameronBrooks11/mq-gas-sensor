/*
  MQGasSensor Library - reading an MQ9

  Demonstrates the use a MQ9 sensor.
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
/************************Hardware Related Macros************************************/
#define         Pin                     (A0)  //Analog input 4 of your arduino
/***********************Software Related Macros************************************/
#define         Type                    ("MQ-9") //MQ9
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ9CleanAir        (9.75) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         PreaheatControlPin5      (3) // Preaheat pin to control with 5 volts
#define         PreaheatControlPin14      (4) // Preaheat pin to control with 1.4 volts
/*****************************Globals***********************************************/
//Declare Sensor
MQGasSensor MQ9(Pin, Voltage_Resolution, ADC_Bit_Resolution, Type);

void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port
  pinMode(PreaheatControlPin5, OUTPUT);
  pinMode(PreaheatControlPin14, OUTPUT);

  //Set math model to calculate the PPM concentration and the value of constants
  MQ9.setCurve(971.09, -2.140); // Configure the equation to to calculate LPG concentration

  /*
    Exponential regression:
  Gas | a          | b
  CH4 | 4224.5     | -2.596
  CO  | 580.02     | -2.220
  LPG | 971.09     | -2.140
  From docs/coefficients.md, the fits to the mq-9-hanwei-2008 datasheet.
  */
  
  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ9.begin(); 
  /* 
    //If the RL value is different from 10K please set it with setLoadResistance:
    MQ9.setLoadResistance(10);
  */
  /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
   // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
  // and on clean air (Calibration conditions), setting up R0 value.
  // We recommend executing this routine only on setup in laboratory conditions.
  // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  // ISSUE 44 - MQ9 needs a low/high temperature cycle like MQ7 #44

  Serial.println("Preheating, please wait 2.5 minutes");
  digitalWrite(PreaheatControlPin5, HIGH);
  digitalWrite(PreaheatControlPin14, LOW);
  delay(60000);
  digitalWrite(PreaheatControlPin5, LOW);
  digitalWrite(PreaheatControlPin14, HIGH);
  delay(90000);
  digitalWrite(PreaheatControlPin5, HIGH);
  digitalWrite(PreaheatControlPin14, LOW);
  delay(60000);
  digitalWrite(PreaheatControlPin5, LOW);
  digitalWrite(PreaheatControlPin14, HIGH);
  delay(90000);
  digitalWrite(PreaheatControlPin5, HIGH);
  digitalWrite(PreaheatControlPin14, LOW);
  
  // End pre-heat cycle

  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ9.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ9.calibrate(RatioMQ9CleanAir);
    Serial.print(".");
  }
  MQ9.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println(F("Warning: R0 is infinite: the sensor output read 0 V. Check that the sensor is powered and AO is wired to the analog pin, not shorted to GND.")); while(1);}
  if(calcR0 == 0){Serial.println(F("Warning: R0 is zero: the sensor output read at or above the supply voltage. Check the supply and ADC reference settings, the divider on a 3.3 V board, and that AO is not shorted to VCC.")); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
}

void loop() {
  digitalWrite(PreaheatControlPin5, LOW);
  digitalWrite(PreaheatControlPin14, HIGH);
  delay(90000);
  digitalWrite(PreaheatControlPin5, HIGH);
  digitalWrite(PreaheatControlPin14, LOW);
  
  MQ9.update(); // Update data, the arduino will read the voltage from the analog pin
  MQ9.readPpm(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  MQ9.printDebug(); // Print one line: voltage, Rs, R0, ratio and ppm
  
}