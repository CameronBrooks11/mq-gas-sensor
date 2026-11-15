#include "MQGasSensor.h"
#include <float.h>
#include <math.h>
#include <string.h>

#define retries 2
#define retry_interval 20

// ---- shared core --------------------------------------------------------------

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
// At Vout = 0 this is infinite, as it always was; readPpm() reports that as NAN.
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

// ---- 2.0 API ------------------------------------------------------------------

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
    _adc = analogRead(_pin);
    avg += _adc;
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
  _adc = raw;
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
  _RS_Calc = rsAt(_VCC - _offset);
  double ratio = (_ratioMode == MQRatio::RsOverR0) ? _RS_Calc / _R0 : _R0 / _RS_Calc;
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

// ---- 1.2 API: the same behaviour as 1.2, through the shared core --------------

MQGasSensor::MQGasSensor(String Placa, float Voltage_Resolution, int ADC_Bit_Resolution, int pin, String type)
{
  _pin = pin;
  Placa.toCharArray(_placa, sizeof(_placa));
  setName(type.c_str());
  _VOLT_RESOLUTION = Voltage_Resolution;
  _VCC = Voltage_Resolution;
  _ADC_Bit_Resolution = ADC_Bit_Resolution;
}
MQGasSensor::MQGasSensor(String Placa, String type)
{
  Placa.toCharArray(_placa, sizeof(_placa));
  setName(type.c_str());
}

void MQGasSensor::init() { begin(); }
void MQGasSensor::externalADCUpdate(float volt) { _sensor_volt = volt; }
void MQGasSensor::setRL(float RL) { _RL = RL; }
void MQGasSensor::setA(float a) { _a = clampCoefficient(a, MQ_MAX_A); }
void MQGasSensor::setB(float b) { _b = clampCoefficient(b, MQ_MAX_B); }
void MQGasSensor::setRegressionMethod(int regressionMethod)
{
  _model = (regressionMethod == 1) ? MQModel::PowerLaw : MQModel::LogLinear;
}
void MQGasSensor::setVoltResolution(float voltage_resolution) { _VOLT_RESOLUTION = voltage_resolution; }
void MQGasSensor::setVCC(float vcc) { _VCC = vcc; }
void MQGasSensor::setPin(int pin) { _pin = pin; }
void MQGasSensor::setADC(int value)
{
  _sensor_volt = voltsFor(value);
  _adc = value;
}

float MQGasSensor::getR0() { return _R0; }
float MQGasSensor::getRL() { return _RL; }
float MQGasSensor::getVoltResolution() { return _VOLT_RESOLUTION; }
float MQGasSensor::getVCC() { return _VCC; }
String MQGasSensor::getRegressionMethod()
{
  return _model == MQModel::PowerLaw ? "Exponential" : "Linear";
}
float MQGasSensor::getA() { return _a; }
float MQGasSensor::getB() { return _b; }

void MQGasSensor::serialDebug(bool onSetup)
{
  if (onSetup)
  {
    Serial.println();
    Serial.println("************************************************************************************************************************************************");
    Serial.println("MQ sensor reading library for arduino");

    Serial.println("Note: remember that all the parameters below can be modified during the program execution with the methods:");
    Serial.println("setR0, setRL, setA, setB where you will have to send as parameter the new value, example: mySensor.setR0(20); //R0 = 20KΩ");

    Serial.println("Authors: Miguel A. Califa U - Yersson R. Carrillo A - Ghiordy F. Contreras C");
    Serial.println("Contributors: Andres A. Martinez - Juan A. Rodríguez - Mario A. Rodríguez O ");

    Serial.print("Sensor: ");
    Serial.println(_name);
    Serial.print("ADC voltage: ");
    Serial.print(_VOLT_RESOLUTION);
    Serial.println(" VDC");
    Serial.print("Sensor supply (VCC): ");
    Serial.print(_VCC);
    Serial.println(" VDC");
    Serial.print("ADC Resolution: ");
    Serial.print(_ADC_Bit_Resolution);
    Serial.println(" Bits");
    Serial.print("R0: ");
    Serial.print(_R0);
    Serial.println(" KΩ");
    Serial.print("RL: ");
    Serial.print(_RL);
    Serial.println(" KΩ");

    Serial.print("Model: ");
    Serial.println(_model == MQModel::PowerLaw ? "Exponential" : "Linear");
    Serial.print(_name);
    Serial.print(" -> a: ");
    Serial.print(_a);
    Serial.print(" | b: ");
    Serial.println(_b);

    Serial.print("Development board: ");
    Serial.println(_placa);
  }
  else
  {
    if (!_firstFlag)
    {
      Serial.print("| ********************************************************************");
      Serial.print(_name);
      Serial.println("*********************************************************************|");
      Serial.println("|ADC_In | Equation_V_ADC | Voltage_ADC |        Equation_RS        | Resistance_RS  |    EQ_Ratio  | Ratio (RS/R0) | Equation_PPM |     PPM    |");
      _firstFlag = true; // Headers are printed
    }
    else
    {
      Serial.print("|");
      Serial.print(_adc);
      Serial.print("| v = ADC*");
      Serial.print(_VOLT_RESOLUTION);
      Serial.print("/");
      Serial.print((pow(2, _ADC_Bit_Resolution)) - 1);
      Serial.print("  |    ");
      Serial.print(_sensor_volt);
      Serial.print("     | RS = ((");
      Serial.print(_VCC);
      Serial.print("*RL)/Voltage) - RL|      ");
      Serial.print(_RS_Calc);
      Serial.print("     | Ratio = RS/R0|    ");
      Serial.print(_ratio);
      Serial.print("       |   ");
      Serial.print(modelFormula(_model)); // 1.2 printed "ratio*a + b" for the power law
      Serial.print("  |   ");
      Serial.print(_PPM);
      Serial.println("  |");
    }
  }
}

float MQGasSensor::validateEcuation(float ratioInput)
{
  if (ratioInput <= 0 || _a == 0)
    return 0;
  _PPM = ppmFor(ratioInput);
  return _PPM;
}

float MQGasSensor::readSensor(bool isMQ303A, float correctionFactor, bool injected)
{
  // 1.2: the MQ-303A flag lowers the supply by 0.45 V, and the correction
  // factor is added to the ratio. readPpm() replaces both.
  float vcc = _VCC - _offset;
  if (isMQ303A)
    vcc = vcc - 0.45;
  _RS_Calc = rsAt(vcc);
  if (!injected)
    _ratio = _RS_Calc / _R0;
  _ratio += correctionFactor;
  if (_ratio <= 0)
    _ratio = 0;
  if (_ratio <= 0 || _a == 0)
    return 0;
  _PPM = ppmFor(_ratio);
  return _PPM;
}

float MQGasSensor::readSensorR0Rs(float correctionFactor)
{
  _RS_Calc = rsAt(_VCC);
  _ratio = _R0 / _RS_Calc; // R0/Rs, for the MQ-131 (upstream issue 28)
  _ratio += correctionFactor;
  if (_ratio <= 0)
    _ratio = 0;
  if (_ratio <= 0 || _a == 0)
    return 0;
  _PPM = ppmFor(_ratio);
  return _PPM;
}

float MQGasSensor::getVoltage(bool read, bool injected, int value)
{
  if (read)
  {
    // 1.2 read the pin here without keeping the voltage; only update() kept it.
    float kept = _sensor_volt;
    update();
    float volts = _sensor_volt;
    _sensor_volt = kept;
    return volts;
  }
  if (!injected)
    return _sensor_volt;
  _sensor_volt = voltsFor(value); // to work on testing
  return _sensor_volt;
}

float MQGasSensor::setRsR0RatioGetPPM(float value)
{
  _ratio = value;
  return readSensor(false, 0, true);
}

float MQGasSensor::getRS()
{
  _RS_Calc = rsAt(_VCC);
  return _RS_Calc;
}

float MQGasSensor::stringTofloat(String &str)
{
  return atof(str.c_str());
}
