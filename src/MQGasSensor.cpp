#include "MQGasSensor.h"
#include <float.h>
#include <math.h>
#include <string.h>

#define retries 2
#define retry_interval 20


static inline double safePow(double base, double exp)
{
  if (exp == 0.0)
    return 1.0;
  if (exp == 1.0)
    return base;
  if (exp == 2.0)
    return base * base;
  return pow(base, exp);
}
static inline bool willOverflow(double log_ppm)
{
  static const double maxLog = log10((double)FLT_MAX);
  static const double minLog = log10((double)FLT_MIN);
  return (log_ppm > maxLog || log_ppm < minLog);
}
static float clampCoefficient(float v, float limit)
{
  if (isinf(v) || isnan(v))
    return 0;
  if (v > limit)
    return limit;
  if (v < -limit)
    return -limit;
  return v;
}
static const char *modelFormula(MQModel model)
{
  return model == MQModel::PowerLaw ? "a*ratio^b" : "10^((log10(ratio)-b)/a)";
}

void MQGasSensor::setName(const char *name)
{
  strncpy(_name, name ? name : "", sizeof(_name) - 1);
  _name[sizeof(_name) - 1] = '\0';
}

float MQGasSensor::voltsFor(int raw) const
{
  return raw * _VOLT_RESOLUTION / ((pow(2, _ADC_Bit_Resolution)) - 1);
}

// Rs = (Vsupply * RL / Vout) - RL, the divider the sensor forms with RL.
// At Vout = 0 this is infinite; readPpm() reports that as NAN.
float MQGasSensor::rsAt(float supplyVolts) const
{
  float rs = ((supplyVolts * _RL) / _sensor_volt) - _RL;
  if (rs < 0)
    rs = 0; // No negative values accepted.
  return rs;
}

float MQGasSensor::ppmFor(double ratio) const
{
  if (ratio <= 0 || _a == 0)
    return 0;
  double logppm = (_model == MQModel::PowerLaw)
                      ? log10((double)_a) + (double)_b * log10(ratio)
                      : (log10(ratio) - (double)_b) / (double)_a;
  double ppm;
  if (willOverflow(logppm))
    ppm = (logppm > 0) ? FLT_MAX : 0.0;
  else
    ppm = safePow(10.0, logppm);
  if (ppm < 0)
    ppm = 0;
  if (isinf(ppm) || isnan(ppm))
    ppm = FLT_MAX;
  return (float)ppm;
}

MQGasSensor::MQGasSensor(uint8_t pin, float adcRefVolts, uint8_t adcBits, const char *name)
{
  _pin = pin;
  _VOLT_RESOLUTION = adcRefVolts;
  _VCC = adcRefVolts;
  _ADC_Bit_Resolution = adcBits;
  setName(name);
}

void MQGasSensor::begin()
{
  pinMode(_pin, INPUT);
}

void MQGasSensor::setLoadResistance(float kOhm) { _RL = kOhm; }
void MQGasSensor::setSupplyVoltage(float volts) { _VCC = volts; }
void MQGasSensor::setAdcReference(float volts) { _VOLT_RESOLUTION = volts; }
void MQGasSensor::setSupplyOffset(float volts) { _offset = volts; }
void MQGasSensor::setRatio(MQRatio ratio) { _ratioMode = ratio; }
void MQGasSensor::setCorrection(float factor) { _correction = factor; }

void MQGasSensor::setCurve(float a, float b, MQModel model)
{
  _a = clampCoefficient(a, MQ_MAX_A);
  _b = clampCoefficient(b, MQ_MAX_B);
  _model = model;
}

float MQGasSensor::calibrate(float cleanAirRatio, float r0Offset)
{
  float R0 = rsAt(_VCC - _offset) / cleanAirRatio; // Rs in clean air over its datasheet ratio
  R0 += r0Offset;
  if (R0 < 0)
    R0 = 0; // No negative values accepted.
  _R0 = R0;
  return R0;
}

void MQGasSensor::setR0(float R0) { _R0 = R0; }

bool MQGasSensor::update()
{
  float avg = 0.0;
  for (int i = 0; i < retries; i++)
  {
    avg += analogRead(_pin);
    delay(retry_interval);
  }
  _sensor_volt = (avg / retries) * _VOLT_RESOLUTION / ((pow(2, _ADC_Bit_Resolution)) - 1);
  return _sensor_volt > 0;
}

bool MQGasSensor::updateFromVoltage(float volts)
{
  _sensor_volt = volts;
  return volts > 0;
}

bool MQGasSensor::updateFromRaw(int raw)
{
  _sensor_volt = voltsFor(raw);
  return _sensor_volt > 0;
}

float MQGasSensor::readPpm()
{
  if (!(_sensor_volt > 0))
  {
    _ratio = NAN;
    _PPM = NAN;
    return NAN;
  }
  float rs = rsAt(_VCC - _offset);
  double ratio = (_ratioMode == MQRatio::RsOverR0) ? rs / _R0 : _R0 / rs;
  ratio /= _correction;
  _ratio = (float)ratio;
  _PPM = ppmFor(ratio);
  return _PPM;
}

float MQGasSensor::ppmFromRatio(float ratio) const { return ppmFor(ratio); }

float MQGasSensor::rs() const { return rsAt(_VCC - _offset); }
float MQGasSensor::ratio() const { return _ratio; }
float MQGasSensor::voltage() const { return _sensor_volt; }
float MQGasSensor::r0() const { return _R0; }
float MQGasSensor::a() const { return _a; }
float MQGasSensor::b() const { return _b; }

void MQGasSensor::printDebug(Print &out)
{
  out.print(_name);
  out.print(": V=");
  out.print(_sensor_volt);
  out.print(" V, Rs=");
  out.print(rs());
  out.print(" kOhm, R0=");
  out.print(_R0);
  out.print(" kOhm, ratio=");
  out.print(_ratio);
  out.print(", ppm=");
  out.print(_PPM);
  out.print(" (");
  out.print(modelFormula(_model));
  out.print(", a=");
  out.print(_a);
  out.print(", b=");
  out.print(_b);
  out.println(")");
}
