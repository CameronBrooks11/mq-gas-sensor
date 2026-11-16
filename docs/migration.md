# Migrating from 1.x

Version 2.0 replaced the API the library inherited from MQSensorsLib. Every 1.x call was removed; this table gives the replacement for each. The [2.0.0 release](https://github.com/CameronBrooks11/mq-gas-sensor/releases/tag/v2.0.0) carries the same notes.

## Call by call

| 1.x | 2.0 |
| --- | --- |
| `MQGasSensor(board, voltRes, adcBits, pin, type)` | `MQGasSensor(pin, adcRefVolts = 5.0, adcBits = 10, name = "MQ")`; no board name |
| `MQGasSensor(board, type)` | `MQGasSensor(pin, …)` with any pin; don't call `begin()` if you never read it |
| `init()` | `begin()` |
| `setRL(kOhm)` | `setLoadResistance(kOhm)` |
| `setVCC(V)` | `setSupplyVoltage(V)` |
| `setVoltResolution(V)` | `setAdcReference(V)` |
| `setRegressionMethod(1); setA(a); setB(b);` | `setCurve(a, b)` |
| `setRegressionMethod(0); setA(a); setB(b);` | `setCurve(a, b, MQModel::LogLinear)` |
| `readSensorR0Rs()` | `setRatio(MQRatio::R0OverRs)`, then `readPpm()` |
| `readSensor(true, …)` (MQ-303A) | `setSupplyOffset(0.45)`, then `readPpm()` |
| `readSensor(false, correctionFactor)` | `setCorrection(factor)`, then `readPpm()` |
| `readSensor()` | `readPpm()` |
| `setR0(calibrate(ratio))` | `calibrate(ratio)`; it stores R0 and returns it. `setR0` remains |
| `externalADCUpdate(V)` | `updateFromVoltage(V)` |
| `setADC(raw)` | `updateFromRaw(raw)` |
| `validateEcuation(r)`, `setRsR0RatioGetPPM(r)` | `ppmFromRatio(r)` |
| `getRS()`, `getVoltage(false)`, `getR0()`, `getA()`, `getB()` | `rs()`, `voltage()`, `r0()`, `a()`, `b()`; also `ratio()` |
| `getRL()`, `getVCC()`, `getVoltResolution()`, `getRegressionMethod()`, `setPin()` | removed; the value is whatever you set |
| `serialDebug(true)`, `serialDebug()` | `printDebug(out = Serial)`: one line per call |
| `stringTofloat(s)` | removed; use `atof(s.c_str())` |

## Behaviour that changed

- **The correction factor divides.** `setCorrection(f)` divides Rs/R0 by `f`, and the default is 1.0. 1.2 added the factor to the ratio. The examples passed 0, except `MQ-135-CorrectionFactorDHT11`, whose factor is near 1.0, so that example roughly doubled every ratio.
- **A 0 V reading is not a reading.** `update()`, `updateFromVoltage()` and `updateFromRaw()` return `false` at 0 V, and `readPpm()` returns `NAN`. 1.2 divided by zero.
- **The MQ-303A offset applies during calibration too.** `setSupplyOffset(0.45)` lowers the supply in every Rs computation, including the one inside `calibrate()`. In 1.2 only the reading subtracted it, so an MQ-303A's R0 will differ from what 1.2 gave.
- **Names are kept up to 11 characters.** 1.2 kept 6, so `"MQ-309A"` became `"MQ-309"`.
- **Debug output is one line**, and names the power law `a*ratio^b`. 1.2 labelled it `ratio*a + b`.

## A 1.x sketch, moved

In 1.x:

```c++
MQGasSensor MQ2("Arduino UNO", 5, 10, A0, "MQ-2");

void setup() {
  MQ2.setRegressionMethod(1);
  MQ2.setA(564.77); MQ2.setB(-2.151);
  MQ2.init();
  MQ2.update();
  MQ2.setR0(MQ2.calibrate(9.51));
}

void loop() {
  MQ2.update();
  float ppm = MQ2.readSensor();
}
```

In 2.0:

```c++
MQGasSensor MQ2(A0, 5.0, 10, "MQ-2");

void setup() {
  MQ2.setCurve(564.77, -2.151);
  MQ2.begin();
  MQ2.update();
  MQ2.calibrate(9.51);
}

void loop() {
  if (MQ2.update()) {
    float ppm = MQ2.readPpm();
  }
}
```
