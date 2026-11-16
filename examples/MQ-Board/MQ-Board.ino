/*
  MQGasSensor Library - calibrating an MQ2 to MQ9

  Demonstrates the use a MQ2 sensor.
  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez
 
  Added example
  modified 27 May 2019
  by Miguel Califa 

  Wiring:
  https://github.com/CameronBrooks11/mq-gas-sensor/blob/main/docs/img/MQ_Arduino.PNG
  Please make sure arduino A0 pin represents the analog input configured on #define pin

 This example code is in the public domain.

*/

//Include the library
#include <MQGasSensor.h>
/************************Hardware Related Macros************************************/
#define         Pin2                     (A2)  //Analog input 2 of your arduino
#define         Pin3                     (A3)  //Analog input 3 of your arduino
#define         Pin4                     (A4)  //Analog input 4 of your arduino
#define         Pin5                     (A5)  //Analog input 5 of your arduino
#define         Pin6                     (A6)  //Analog input 6 of your arduino
#define         Pin7                     (A7)  //Analog input 7 of your arduino
#define         Pin8                     (A8)  //Analog input 8 of your arduino
#define         Pin9                     (A9)  //Analog input 9 of your arduino
#define         PWMPin                   (5)   // Pin connected to mosfet
/***********************Software Related Macros************************************/
#define         RatioMQ2CleanAir          (9.51) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         RatioMQ3CleanAir          (59.9) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         RatioMQ4CleanAir          (4.43) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         RatioMQ5CleanAir          (6.49) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         RatioMQ6CleanAir          (9.86) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         RatioMQ7CleanAir          (26.1) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         RatioMQ8CleanAir          (69.5) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         RatioMQ9CleanAir          (9.75) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         ADC_Bit_Resolution        (10) // 10 bit ADC 
#define         Voltage_Resolution        (5) // Volt resolution to calc the voltage
/*****************************Globals***********************************************/
//Declare Sensor
MQGasSensor MQ2(Pin2, Voltage_Resolution, ADC_Bit_Resolution, "MQ-2");
MQGasSensor MQ3(Pin3, Voltage_Resolution, ADC_Bit_Resolution, "MQ-3");
MQGasSensor MQ4(Pin4, Voltage_Resolution, ADC_Bit_Resolution, "MQ-4");
MQGasSensor MQ5(Pin5, Voltage_Resolution, ADC_Bit_Resolution, "MQ-5");
MQGasSensor MQ6(Pin6, Voltage_Resolution, ADC_Bit_Resolution, "MQ-6");
MQGasSensor MQ7(Pin7, Voltage_Resolution, ADC_Bit_Resolution, "MQ-7");
MQGasSensor MQ8(Pin8, Voltage_Resolution, ADC_Bit_Resolution, "MQ-8");
MQGasSensor MQ9(Pin9, Voltage_Resolution, ADC_Bit_Resolution, "MQ-9");

unsigned long oldTime = 0;

void setup() {
  //Init serial port
  Serial.begin(9600);
  //init the sensor
  MQ2.begin();
  MQ2.setCurve(564.77, -2.151); // Configure the equation to to calculate LPG concentration
  MQ2.setR0(9.659574468);

  MQ3.begin(); 
  MQ3.setCurve(0.39304, -1.495); //Configure the equation to calculate alcohol concentration value
  MQ3.setR0(3.86018237);

  MQ4.begin(); 
  MQ4.setCurve(990.22, -2.826); // Configure the equation to to calculate CH4 concentration
  MQ4.setR0(3.86018237);

  MQ5.begin(); 
  MQ5.setCurve(67573, -4.526); //Configure the equation to calculate alcohol concentration value
  MQ5.setR0(71.100304);
  
  MQ6.begin(); 
  MQ6.setCurve(2195.3, -2.507); // Configure the equation to to calculate CH4 concentration
  MQ6.setR0(13.4285714);
  
  MQ7.begin(); 
  MQ7.setCurve(98.691, -1.528); // Configure the equation to calculate CO concentration value
  MQ7.setR0(4);
    
  MQ8.begin(); 
  MQ8.setCurve(973.62, -0.6902); // Configure the equation to to calculate H2 concentration
  MQ8.setR0(1);
  
  MQ9.begin(); 
  MQ9.setCurve(971.09, -2.140); // Configure the equation to to calculate LPG concentration
  MQ9.setR0(9.42857143);

   /*****************************  MQ CAlibration ********************************************/ 
  // Explanation: 
   // In this routine the sensor will measure the resistance of the sensor supposedly before being pre-heated
  // and on clean air (Calibration conditions), setting up R0 value.
  // We recommend executing this routine only on setup in laboratory conditions.
  // This routine does not need to be executed on each restart, you can load your R0 value from eeprom.
  // Acknowledgements: https://jayconsystems.com/blog/understanding-a-gas-sensor
  Serial.print("Calibrating please wait.");
  float MQ2calcR0 = 0, 
  MQ3calcR0 = 0,
  MQ4calcR0 = 0,
  MQ5calcR0 = 0,
  MQ6calcR0 = 0,
  MQ7calcR0 = 0,
  MQ8calcR0 = 0,
  MQ9calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    //Update the voltage Values
    MQ2.update();
    MQ3.update();
    MQ4.update();
    MQ5.update();
    MQ6.update();
    MQ7.update();
    MQ8.update();
    MQ9.update();

    MQ2calcR0 += MQ2.calibrate(RatioMQ2CleanAir);
    MQ3calcR0 += MQ3.calibrate(RatioMQ3CleanAir);
    MQ4calcR0 += MQ4.calibrate(RatioMQ4CleanAir);
    MQ5calcR0 += MQ5.calibrate(RatioMQ5CleanAir);
    MQ6calcR0 += MQ6.calibrate(RatioMQ6CleanAir);
    MQ7calcR0 += MQ7.calibrate(RatioMQ7CleanAir);
    MQ8calcR0 += MQ8.calibrate(RatioMQ8CleanAir);
    MQ9calcR0 += MQ9.calibrate(RatioMQ9CleanAir);

    Serial.print(".");
  }
  MQ2.setR0(MQ2calcR0/10);
  MQ3.setR0(MQ3calcR0/10);
  MQ4.setR0(MQ4calcR0/10);
  MQ5.setR0(MQ5calcR0/10);
  MQ6.setR0(MQ6calcR0/10);
  MQ7.setR0(MQ7calcR0/10);
  MQ8.setR0(MQ8calcR0/10);
  MQ9.setR0(MQ9calcR0/10);
  Serial.println("  done!.");
  
  Serial.print("Valores de R0 para cada sensor (MQ2 - MQ9):");
  Serial.print(MQ2calcR0/10); Serial.print(" | ");
  Serial.print(MQ3calcR0/10); Serial.print(" | ");
  Serial.print(MQ4calcR0/10); Serial.print(" | ");
  Serial.print(MQ5calcR0/10); Serial.print(" | ");
  Serial.print(MQ6calcR0/10); Serial.print(" | ");
  Serial.print(MQ7calcR0/10); Serial.print(" | ");
  Serial.print(MQ8calcR0/10); Serial.print(" | ");
  Serial.print(MQ9calcR0/10); Serial.println(" |");

  if(isinf(MQ2calcR0) || isinf(MQ3calcR0) || isinf(MQ4calcR0) || isinf(MQ5calcR0)  || isinf(MQ6calcR0) || isinf(MQ7calcR0) || isinf(MQ8calcR0) || isinf(MQ9calcR0)) {Serial.println(F("Warning: an R0 is infinite: that sensor's output read 0 V. Check that it is powered and its AO is wired to its analog pin, not shorted to GND.")); while(1);}
  if(MQ2calcR0 == 0 || MQ3calcR0 == 0 || MQ4calcR0 == 0 || MQ5calcR0 == 0 || MQ6calcR0 == 0 || MQ7calcR0 == 0 || MQ8calcR0 == 0 || MQ9calcR0 == 0){Serial.println(F("Warning: an R0 is zero: that sensor's output read at or above the supply voltage. Check the supply and ADC reference settings, and that its AO is not shorted to VCC.")); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
 
  //Print in serial monitor
  Serial.println("MQ2 to MQ9 - lecture");
  Serial.println("*************************** Values from MQ-board ***************************");
  Serial.println("|    LPG   |  Alcohol |   CH4    |  Alcohol |   CH4    |    CO    |    H2    |    LPG   |");  
  Serial.println("|    MQ-2  |   MQ-3   |   MQ-4   |   MQ-5   |   MQ-6   |   MQ-7   |   MQ-8   |   MQ-9   |");  
  Serial.println("MQ-3 in mg/L, the unit of its datasheet's figure; the others in ppm.");
  //pinMode(calibration_button, INPUT);
}

void loop() {
  oldTime = millis();
  while(millis() - oldTime <= (60*1000))
  { 
    // VH 5 Volts
    analogWrite(5, 255); // 255 is DC 5V output
    readAllSensors();
    delay(500);
  }
  // 90s cycle
  oldTime = millis();
  while(millis() - oldTime <= (90*1000))
  {
    // VH 1.4 Volts
    analogWrite(5, 20); // 255 is 100%, 20.4 is aprox 8% of Duty cycle for 90s
    readAllSensors();
    delay(500);
  }
}

void readAllSensors()
{
  //Update the voltage Values
  MQ2.update();
  MQ3.update();
  MQ4.update();
  MQ5.update();
  MQ6.update();
  MQ7.update();
  MQ8.update();
  MQ9.update();
  //Read the sensor and print in serial port
  float MQ2Lecture =  MQ2.readPpm();
  float MQ3Lecture =  MQ3.readPpm();
  float MQ4Lecture =  MQ4.readPpm();
  float MQ5Lecture =  MQ5.readPpm();
  float MQ6Lecture =  MQ6.readPpm();
  float MQ7Lecture =  MQ7.readPpm();
  float MQ8Lecture =  MQ8.readPpm();
  float MQ9Lecture =  MQ9.readPpm();

  Serial.print("| "); Serial.print(MQ2Lecture);
  Serial.print("   |   "); Serial.print(MQ3Lecture);
  Serial.print("   |   "); Serial.print(MQ4Lecture);
  Serial.print("   |   "); Serial.print(MQ5Lecture);
  Serial.print("   |   "); Serial.print(MQ6Lecture);
  Serial.print("   |   "); Serial.print(MQ7Lecture);
  Serial.print("   |   "); Serial.print(MQ8Lecture);
  Serial.print("   |   "); Serial.print(MQ9Lecture);
  Serial.println("|");
}