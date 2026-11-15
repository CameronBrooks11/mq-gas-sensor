/*
  MQGasSensor Library - testing library

  Library originally added 01 may 2019
  by Miguel A Califa, Yersson Carrillo, Ghiordy Contreras, Mario Rodriguez
 
  Added example
  modified 21 March 2022
  by Miguel Califa 

 This example code is in the public domain.

*/

//Include the library
#include <MQGasSensor.h>
#line 2 "basic.ino"
#include <ArduinoUnit.h>

/************************Hardware Related Macros************************************/
#define         Board                   ("Arduino Mega")
#define         Pin2                     (A2)  //Analog input 2 of your arduino
#define         Pin3                     (A3)  //Analog input 3 of your arduino
#define         Pin4                     (A4)  //Analog input 4 of your arduino
#define         Pin5                     (A5)  //Analog input 5 of your arduino
#define         Pin6                     (A6)  //Analog input 6 of your arduino
#define         Pin7                     (A7)  //Analog input 7 of your arduino
#define         Pin8                     (A8)  //Analog input 8 of your arduino
#define         Pin9                     (A9)  //Analog input 9 of your arduino
#define         Pin10                    (A10) //Analog input 10 of your arduino
#define         Pin11                    (A11) //Analog input 11 of your arduino
#define         Pin12                    (A12) //Analog input 12 of your arduino
#define         Pin13                    (A13) //Analog input 13 of your arduino
#define         Pin14                    (A14) //Analog input 14 of your arduino
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
#define         RatioMQ136CleanAir        (3.55) // Rs/R0 in clean air, from the datasheet's AIR line (docs/coefficients.md)
#define         RatioMQ131CleanAir        (1.0 / 12.0) // Rs/R0 in clean air: the MQ-131 figure gives R0/Rs 12.0
#define         ADC_Bit_Resolution        (10) // 10 bit ADC 
#define         Voltage_Resolution        (5) // Volt resolution to calc the voltage
#define         Type                      ("Arduino Mega 2560") //Board used
/*****************************Globals***********************************************/
//Declare Sensor
MQGasSensor MQ2(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin2, Type);
MQGasSensor MQ3(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin3, Type);
MQGasSensor MQ4(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin4, Type);
MQGasSensor MQ5(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin5, Type);
MQGasSensor MQ6(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin6, Type);
MQGasSensor MQ7(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin7, Type);
MQGasSensor MQ8(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin8, Type);
MQGasSensor MQ9(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin9, Type);
MQGasSensor MQ131(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin10, Type); //mq131
MQGasSensor MQ135(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin11, Type); //mq135
MQGasSensor MQ136(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin12, Type); //mq136
MQGasSensor MQ303A(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin13, Type); //mq303
MQGasSensor MQ309A(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin14, Type); //mq309


test(adcMaxValue)
{
  int adcVal=1023;
  int volt = MQ2.getVoltage(false, true, adcVal);
  int expVolt = 5;
  assertEqual(volt,expVolt);
}

test(adcMinValue)
{
  int adcVal=0;
  int volt = MQ2.getVoltage(false, true, adcVal);
  int expVolt = 0;
  assertEqual(volt,expVolt);
}

test(adcMidValue)
{
  int adcVal=410;
  int volt = MQ2.getVoltage(false, true, adcVal);
  int expVolt = 2;
  assertEqual(volt,expVolt);
}

test(MQ2_LPG)
{
  MQ2.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ2.setA(564.77); MQ2.setB(-2.151); // Configure the equation to to calculate LPG concentration
  MQ2.init(); 
  MQ2.setR0(10);
  
  int ppmExp=460;
  int PPM = MQ2.setRsR0RatioGetPPM(1.1); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}

test(MQ3_Alcohol)
{
  MQ3.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ3.setA(0.39304); MQ3.setB(-1.495); // Configure the equation to to calculate alcohol concentration
  MQ3.init(); 
  MQ3.setR0(10);
  
  int ppmExp=4; // mg/L: the MQ-3 coefficients are in the unit of its datasheet's figure
  int PPM = MQ3.setRsR0RatioGetPPM(0.2); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}


test(MQ4_Smoke)
{
  MQ4.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ4.setA(4.7509e+07); MQ4.setB(-8.836); // Configure the equation to to calculate smoke concentration
  MQ4.init(); 
  MQ4.setR0(10);
  
  int ppmExp=227;
  int PPM = MQ4.setRsR0RatioGetPPM(4); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}

test(MQ5_LPG)
{
  MQ5.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ5.setA(77.016); MQ5.setB(-2.501); // Configure the equation to to calculate LPG concentration
  MQ5.init(); 
  MQ5.setR0(10);
  
  int ppmExp=187;
  int PPM = MQ5.setRsR0RatioGetPPM(0.7); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}

test(MQ6_CH4)
{
  MQ6.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ6.setA(2195.3); MQ6.setB(-2.507); // Configure the equation to to calculate CH4 concentration
  MQ6.init(); 
  MQ6.setR0(10);
  
  int ppmExp=2195;
  int PPM = MQ6.setRsR0RatioGetPPM(1); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}

test(MQ7_CO)
{
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ7.setA(98.691); MQ7.setB(-1.528); // Configure the equation to to calculate CO concentration
  MQ7.init(); 
  MQ7.setR0(10);
  
  int ppmExp=1154;
  int PPM = MQ7.setRsR0RatioGetPPM(0.2); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}

test(MQ8_H2)
{
  MQ8.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ8.setA(973.62); MQ8.setB(-0.6902); // Configure the equation to to calculate H2 concentration
  MQ8.init(); 
  MQ8.setR0(10);
  
  int ppmExp=2956;
  int PPM = MQ8.setRsR0RatioGetPPM(0.2); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}


test(MQ9_CO)
{
  MQ9.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ9.setA(580.02); MQ9.setB(-2.220); // Configure the equation to to calculate CO concentration
  MQ9.init(); 
  MQ9.setR0(10);
  
  int ppmExp=951;
  int PPM = MQ9.setRsR0RatioGetPPM(0.8); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}

test(MQ131_O3)
{    
  MQ131.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ131.setA(24.264); MQ131.setB(-1.117); // Configure the equation to to calculate O3 concentration
  MQ131.init(); 
  MQ131.setR0(10);  
  
  int ppmExp=11;
  int PPM = MQ131.setRsR0RatioGetPPM(2); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}


test(MQ131_CleanAir)
{
  // Calibrate in clean air as examples/MQ-131 does, then read the same air.
  MQ131.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ131.setA(24.264); MQ131.setB(-1.117);
  MQ131.init();
  MQ131.externalADCUpdate(1.0);
  MQ131.setR0(MQ131.calibrate(RatioMQ131CleanAir));

  float PPM = MQ131.readSensorR0Rs();

  assertLess(PPM, 5); // below 5 ppm, the lowest concentration the figure plots
}

test(MQ135_CO2)
{
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(109.66); MQ135.setB(-2.856); // Configure the equation to to calculate CO2 concentration
  MQ135.init(); 
  MQ135.setR0(10);
  
  int ppmExp=109;
  int PPM = MQ135.setRsR0RatioGetPPM(1); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}

test(MQ136_H2S)
{
  MQ136.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ136.setA(36.716); MQ136.setB(-3.481); // Configure the equation to to calculate H2S concentration
  MQ136.init(); 
  MQ136.setR0(10);
  
  int ppmExp=217;
  int PPM = MQ136.setRsR0RatioGetPPM(0.6); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}

test(MQ303A_Ethanol)
{
  MQ303A.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ303A.setA(3.5821); MQ303A.setB(-2.423); // Configure the equation to to calculate ethanol concentration
  MQ303A.init();
  MQ303A.setR0(10);

  int ppmExp=948;
  int PPM = MQ303A.setRsR0RatioGetPPM(0.1); // Send and Rs/R0 and return PPM (Using datasheet)

  assertEqual(PPM,ppmExp);
}

test(MQ303A_VoltResolution)
{
  float vRes = 4.7;
  MQ303A.setVoltResolution(vRes);
  MQ303A.setADC(100);            // provide dummy ADC value
  float correctionFactor = 0; // Optional environmental correction
  MQ303A.readSensor(true, correctionFactor);       // dummy read for MQ303A
  assertEqual(MQ303A.getVoltResolution(), vRes);
}

test(MQ303A_VoltageOffset)
{
  MQ303A.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ303A.setA(1); MQ303A.setB(1); // PPM equals the ratio RS/R0
  MQ303A.setR0(1);                // ... which equals RS
  MQ303A.setRL(10);
  MQ303A.setVoltResolution(5);
  MQ303A.setVCC(5);
  MQ303A.setADC(512);
  float volt = 512 * 5.0 / 1023;

  float plain = MQ303A.readSensor(false);
  float offset = MQ303A.readSensor(true);

  // An MQ303A read lowers the supply voltage by 0.45 V in RS = (VCC*RL/volt) - RL
  assertNear(plain - offset, 0.45 * 10 / volt, 0.01);
}

test(MQ309A_CO)
{
  MQ309A.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ309A.setA(1.6313e+06); MQ309A.setB(-4.076); // Configure the equation to to calculate CO concentration
  MQ309A.init(); 
  MQ309A.setR0(10);
  
  int ppmExp=1098;
  int PPM = MQ309A.setRsR0RatioGetPPM(6); // Send and Rs/R0 and return PPM (Using datasheet)
  
  assertEqual(PPM,ppmExp);
}


/*********************** 2.0 API ************************************************/
// Each test builds its own sensor, so no state carries from one to the next.
// RL 10 kOhm and a 5 V supply: at 1.0 V out, Rs = 5*10/1 - 10 = 40 kOhm.

// Collects what printDebug writes, so a test can look for text in it.
class Capture : public Print
{
public:
  char text[200] = "";
  size_t length = 0;
  size_t write(uint8_t c) override
  {
    if (length < sizeof(text) - 1)
    {
      text[length++] = c;
      text[length] = '\0';
    }
    return 1;
  }
  bool has(const char *s) const { return strstr(text, s) != NULL; }
};

test(v2_name_is_kept_whole)
{
  MQGasSensor s(A15, 5.0, 10, "MQ-309A");
  Capture out;
  s.printDebug(out);
  assertTrue(out.has("MQ-309A:")); // 1.2 kept 6 characters: "MQ-309"
}

test(v2_debug_names_the_power_law)
{
  MQGasSensor s(A15);
  s.setCurve(990.22, -2.826, MQModel::PowerLaw);
  Capture out;
  s.printDebug(out);
  assertTrue(out.has("a*ratio^b")); // 1.2 printed "ratio*a + b"
}

test(v2_power_law_curve)
{
  MQGasSensor s(A15);
  s.setCurve(990.22, -2.826);
  assertNear(s.ppmFromRatio(1.0), 990.22, 0.01);
  assertNear(s.ppmFromRatio(0.5), 990.22 * pow(0.5, -2.826), 0.5);
}

test(v2_log_linear_curve)
{
  MQGasSensor s(A15);
  s.setCurve(-0.38, 1.48, MQModel::LogLinear);
  assertNear(s.ppmFromRatio(2.0), pow(10, (log10(2.0) - 1.48) / -0.38), 0.01);
}

test(v2_calibrate_stores_r0)
{
  MQGasSensor s(A15);
  s.setLoadResistance(10);
  s.updateFromVoltage(1.0);           // Rs = 40
  float r0 = s.calibrate(4.0);        // R0 = Rs / 4
  assertNear(r0, 10.0, 0.001);
  assertNear(s.r0(), 10.0, 0.001);    // 1.2 returned R0 without keeping it
}

test(v2_correction_divides_the_ratio)
{
  MQGasSensor s(A15);
  s.setLoadResistance(10);
  s.setR0(10);
  s.setCurve(990.22, -2.826);
  s.updateFromVoltage(1.0);           // Rs/R0 = 4
  s.setCorrection(2.0);
  s.readPpm();
  assertNear(s.ratio(), 2.0, 0.0001); // 1.2 added the factor: 6
}

test(v2_r0_over_rs)
{
  MQGasSensor s(A15);
  s.setLoadResistance(10);
  s.setR0(10);
  s.setCurve(990.22, -2.826);
  s.setRatio(MQRatio::R0OverRs);
  s.updateFromVoltage(1.0);           // R0/Rs = 10/40
  s.readPpm();
  assertNear(s.ratio(), 0.25, 0.0001);
}

test(v2_supply_offset)
{
  MQGasSensor s(A15);
  s.setLoadResistance(10);
  s.setSupplyOffset(0.45);            // the MQ-303A
  s.updateFromVoltage(1.0);           // Rs = (5 - 0.45)*10/1 - 10
  assertNear(s.rs(), 35.5, 0.001);
}

test(v2_zero_volts_is_no_reading)
{
  MQGasSensor s(A15);
  s.setR0(10);
  s.setCurve(990.22, -2.826);
  assertFalse(s.updateFromVoltage(0.0));
  assertTrue(isnan(s.readPpm()));     // 1.2 divided by zero and returned 0
  assertFalse(s.updateFromRaw(0));
  assertTrue(s.updateFromRaw(512));
  assertNear(s.voltage(), 512 * 5.0 / 1023, 0.001);
}

void setup()
{
  Serial.begin(9600);
  while(!Serial) {} // Portability for Leonardo/Micro
}

void loop()
{
  Test::run();
}
