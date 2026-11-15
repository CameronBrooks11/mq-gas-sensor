#ifndef MQGasSensor_H
#define MQGasSensor_H

#include <Arduino.h>
#include <stdint.h>
#include <float.h>
#include <math.h>

// Maximum coefficients allowed for the regression model
#define MQ_MAX_A 1e30
#define MQ_MAX_B 100.0

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

private:
  float rsAt(float supplyVolts) const; // Rs for the last voltage; INFINITY at 0 V
  float ppmFor(double ratio) const;    // the curve, with its overflow guards
  float voltsFor(int raw) const;       // a raw ADC value in volts

  void setName(const char *name);

  byte _pin = 1;
  float _VOLT_RESOLUTION = 5.0; // ADC reference
  float _VCC = 5.0;             // Sensor supply voltage
  float _offset = 0.0;          // subtracted from the supply in Rs
  float _correction = 1.0;      // the ratio is divided by it
  float _RL = 10;               // Value in KiloOhms
  byte _ADC_Bit_Resolution = 10;
  MQModel _model = MQModel::PowerLaw;
  MQRatio _ratioMode = MQRatio::RsOverR0;

  float _a = 0, _b = 0, _sensor_volt = 0;
  float _R0 = 0, _ratio = 0, _PPM = 0;

  char _name[12] = "MQ";
};

#endif // MQGasSensor_H
