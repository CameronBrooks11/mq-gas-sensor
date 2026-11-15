#ifndef MQGasSensor_H
#define MQGasSensor_H

#include <Arduino.h>
#include <stdint.h>
#include <float.h>
#include <math.h>

// Maximum coefficients allowed for the regression model
#define MQ_MAX_A 1e30
#define MQ_MAX_B 100.0

#define MQ_DEPRECATED(use) __attribute__((deprecated("use " use)))

// How a curve turns a sensor ratio into a concentration.
enum class MQModel : uint8_t
{
  PowerLaw,  // ppm = a * ratio^b
  LogLinear, // ppm = 10^((log10(ratio) - b) / a)
};

// Which ratio a curve is plotted against. Most datasheets plot Rs/R0; the
// MQ-131 plots R0/Rs.
enum class MQRatio : uint8_t
{
  RsOverR0,
  R0OverRs,
};

class MQGasSensor
{
public:
  // ---- 2.0 API ----------------------------------------------------------------

  // pin: the analog pin the sensor's output is wired to.
  // adcRefVolts: the ADC's reference voltage; adcBits: its resolution.
  // name: shown by printDebug, such as "MQ-4"; up to 11 characters are kept.
  MQGasSensor(uint8_t pin, float adcRefVolts = 5.0, uint8_t adcBits = 10, const char *name = "MQ");

  void begin();

  void setLoadResistance(float kOhm);  // RL, in kilohms (default 10)
  void setSupplyVoltage(float volts);  // the voltage across the sensor and RL (default: the ADC reference)
  void setAdcReference(float volts);   // the ADC reference voltage
  void setSupplyOffset(float volts);   // subtracted from the supply in Rs; the MQ-303A needs 0.45
  void setCurve(float a, float b, MQModel model = MQModel::PowerLaw);
  void setRatio(MQRatio ratio);        // default MQRatio::RsOverR0
  void setCorrection(float factor);    // the ratio is divided by it before the curve (default 1.0)

  // Measures Rs now, sets R0 = Rs / cleanAirRatio and returns it. cleanAirRatio
  // is the datasheet's Rs/R0 in clean air.
  float calibrate(float cleanAirRatio, float r0Offset = 0.0);
  void setR0(float kOhm = 10);

  bool update();                  // read the analog pin; false if it reads 0 V
  bool updateFromVoltage(float volts); // use a voltage measured elsewhere; false if 0 V
  bool updateFromRaw(int raw);    // use a raw ADC value measured elsewhere; false if 0

  float readPpm();                // NAN if the last update read 0 V
  float ppmFromRatio(float ratio) const;

  float rs() const;               // kilohms, from the last update
  float ratio() const;            // the ratio the last readPpm used
  float voltage() const;          // volts, from the last update
  float r0() const;
  float a() const;
  float b() const;

  void printDebug(Print &out = Serial);

  // ---- 1.2 API, kept until the examples move to 2.0 -------------------------

  MQGasSensor(String Placa = "Arduino", float Voltage_Resolution = 5, int ADC_Bit_Resolution = 10, int pin = 1, String type = "CUSTOM MQ");
  MQGasSensor(String Placa = "Arduino", String type = "CUSTOM MQ");

  MQ_DEPRECATED("begin()") void init();
  MQ_DEPRECATED("updateFromVoltage()") void externalADCUpdate(float volt);
  MQ_DEPRECATED("setLoadResistance()") void setRL(float RL = 10);
  MQ_DEPRECATED("setCurve()") void setA(float a);
  MQ_DEPRECATED("setCurve()") void setB(float b);
  MQ_DEPRECATED("setCurve()") void setRegressionMethod(int regressionMethod);
  MQ_DEPRECATED("setAdcReference()") void setVoltResolution(float voltage_resolution = 5);
  MQ_DEPRECATED("setSupplyVoltage()") void setVCC(float vcc = 5);
  MQ_DEPRECATED("the pin constructor") void setPin(int pin = 1);
  MQ_DEPRECATED("printDebug()") void serialDebug(bool onSetup = false);
  MQ_DEPRECATED("updateFromRaw()") void setADC(int value);
  MQ_DEPRECATED("readPpm()") float readSensor(bool isMQ303A = false, float correctionFactor = 0.0, bool injected = false);
  MQ_DEPRECATED("setRatio(MQRatio::R0OverRs) and readPpm()") float readSensorR0Rs(float correctionFactor = 0.0);
  MQ_DEPRECATED("ppmFromRatio()") float validateEcuation(float ratioInput = 0);
  MQ_DEPRECATED("a()") float getA();
  MQ_DEPRECATED("b()") float getB();
  MQ_DEPRECATED("r0()") float getR0();
  MQ_DEPRECATED("nothing; RL is set by setLoadResistance()") float getRL();
  MQ_DEPRECATED("nothing; the reference is set once") float getVoltResolution();
  MQ_DEPRECATED("nothing; the supply is set by setSupplyVoltage()") float getVCC();
  MQ_DEPRECATED("nothing; the model is set by setCurve()") String getRegressionMethod();
  MQ_DEPRECATED("voltage()") float getVoltage(bool read = true, bool injected = false, int value = 0);
  MQ_DEPRECATED("atof()") float stringTofloat(String &str);
  MQ_DEPRECATED("rs()") float getRS();
  MQ_DEPRECATED("ppmFromRatio()") float setRsR0RatioGetPPM(float value);

private:
  // The one place each quantity is computed; both APIs go through these.
  float rsAt(float supplyVolts) const; // Rs for the last voltage; INFINITY at 0 V
  float ppmFor(double ratio) const;    // the curve, with its overflow guards
  float voltsFor(int raw) const;       // a raw ADC value in volts

  void setName(const char *name);

  byte _pin = 1;
  byte _firstFlag = false;
  float _VOLT_RESOLUTION = 5.0; // ADC reference
  float _VCC = 5.0;             // Sensor supply voltage
  float _offset = 0.0;          // subtracted from the supply in Rs
  float _correction = 1.0;      // the 2.0 ratio is divided by it
  float _RL = 10;               // Value in KiloOhms
  byte _ADC_Bit_Resolution = 10;
  MQModel _model = MQModel::PowerLaw;
  MQRatio _ratioMode = MQRatio::RsOverR0;

  float _adc = 0, _a = 0, _b = 0, _sensor_volt = 0;
  float _R0 = 0, _ratio = 0, _PPM = 0, _RS_Calc = 0;

  char _name[12] = "MQ";  // 7 bytes in 1.2 cut "MQ-309A" to "MQ-309"
  char _placa[20] = "";   // 1.2 only: the board name serialDebug prints
};

#endif // MQGasSensor_H
