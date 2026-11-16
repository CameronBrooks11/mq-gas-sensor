/*
  MQGasSensor Library - reading an MQ135

  Demonstrates the use a MQ135 sensor.
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
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//Definitions
#define Voltage_Resolution 5
#define pin A0 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.60 // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
//#define calibration_button 13 //Pin to calibrate your sensor

#define MQ135_DEFAULTPPM 399 //default ppm of CO2 for calibration
#define MQ135_DEFAULTRO 68550 //default Ro for MQ135_DEFAULTPPM ppm of CO2
#define MQ135_SCALINGFACTOR 116.6020682 //CO2 gas value
#define MQ135_EXPONENT -2.769034857 //CO2 gas value
#define MQ135_MAXRSRO 2.428 //for CO2
#define MQ135_MINRSRO 0.358 //for CO2


/// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA 116.6020682
#define PARB 2.769034857

/// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

#define DHTPIN 2     // Digital pin connected to the DHT sensor 
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

//Declare Sensor
MQGasSensor MQ135(pin, Voltage_Resolution, ADC_Bit_Resolution, type);

void setup() {
  //Init the serial port communication - to debug the library
  Serial.begin(9600); //Init serial port
  dht.begin();
  sensor_t sensor;
  
  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setCurve(102.29, -2.484); // Configure the equation to to calculate NH4 concentration

  /*
    Exponential regression:
  Gas     | a          | b
  acetone | 34.544     | -3.402
  alcohol | 76.939     | -3.173
  CO      | 610.01     | -3.960
  CO2     | 109.66     | -2.856
  NH4     | 102.29     | -2.484
  toluene | 44.880     | -3.443
  From docs/coefficients.md, the fits to the mq-135-hanwei datasheet.
  */
  
  /*****************************  MQ Init ********************************************/ 
  //Remarks: Configure the pin of arduino as input.
  /************************************************************************************/ 
  MQ135.begin(); 
  /* 
    //If the RL value is different from 10K please set it with setLoadResistance:
    MQ135.setLoadResistance(10);
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
    MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println(F("Warning: R0 is infinite: the sensor output read 0 V. Check that the sensor is powered and AO is wired to the analog pin, not shorted to GND.")); while(1);}
  if(calcR0 == 0){Serial.println(F("Warning: R0 is zero: the sensor output read at or above the supply voltage. Check the supply and ADC reference settings, the divider on a 3.3 V board, and that AO is not shorted to VCC.")); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  // Set delay between sensor readings based on sensor details.
  delayMS = sensor.min_delay / 1000;
}

void loop() {
  
  delay(delayMS); //Sampling frequency

  // if you want to apply corelation factor, you will add in this program the temperature and humidity sensor
  sensors_event_t temperature, humidity;
  dht.temperature().getEvent(&temperature);
  dht.humidity().getEvent(&humidity);
  float cFactor = 1.0; // no correction
  if (!isnan(temperature.temperature) && !isnan(humidity.relative_humidity)) cFactor = getCorrectionFactor(temperature.temperature, humidity.relative_humidity);
  Serial.print("Correction Factor: "); Serial.println(cFactor);
  MQ135.setCorrection(cFactor); // Rs/R0 is divided by it, as in getCorrectedResistance below
  MQ135.update(); // Update data, the arduino will read the voltage from the analog pin
  MQ135.readPpm(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup
  MQ135.printDebug(); // Print one line: voltage, Rs, R0, ratio and ppm
  
}

/**************************************************************************/
/*!
@brief  Get the correction factor to correct for temperature and humidity
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The calculated correction factor
*/
/**************************************************************************/
float getCorrectionFactor(float t, float h) {
  return CORA * t * t - CORB * t + CORC - (h-33.)*CORD;
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value corrected
        for temp/hum
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The corrected sensor resistance kOhm
*/
/**************************************************************************/
float getCorrectedResistance(long resvalue, float t, float h) {
  return resvalue/getCorrectionFactor(t, h);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
        for temp/hum
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The ppm of CO2 in the air
*/
/**************************************************************************/
float getCorrectedPPM(long resvalue,float t, float h, long ro) {
  return PARA * pow((getCorrectedResistance(resvalue, t, h)/ro), -PARB);
}
