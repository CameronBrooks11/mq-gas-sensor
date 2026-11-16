# MQGasSensor

An Arduino library that turns the analog output of an MQ-series gas sensor into a gas concentration. It measures the sensor's resistance Rs, calibrates its clean-air resistance R0, and converts the ratio Rs/R0 to ppm with a curve fitted from the sensor's datasheet.

```c++
#include <MQGasSensor.h>

MQGasSensor MQ4(A4, 5.0, 10, "MQ-4"); // analog pin, ADC reference (V), ADC bits, name

void setup() {
  Serial.begin(9600);
  MQ4.begin();
  MQ4.setCurve(990.22, -2.826); // CH4, from the MQ-4 datasheet
  MQ4.update();                 // in clean air, after the heater has warmed up
  MQ4.calibrate(4.43);          // the datasheet's Rs/R0 in clean air
}

void loop() {
  if (MQ4.update()) Serial.println(MQ4.readPpm());
  delay(500);
}
```

## What sets it apart

- **Every coefficient is traced to a datasheet.** Each `a` and `b` the examples pass to `setCurve` is fitted from a named datasheet figure, through a recipe anyone can re-run. [Calibration coefficients](coefficients.md) lists them; [Methods](methods.md) describes how they are made.
- **Checked on every change.** CI compiles each example for its board (AVR, ESP32, ESP8266) and runs the unit tests on a simulated Arduino Mega.

!!! warning "Not yet validated against physical sensors"
    The coefficients describe each datasheet's *typical* curve. No reading from this library has been checked against a reference instrument. Do not rely on it where a wrong reading could harm someone.

## Where to go next

- [Getting started](getting-started.md): install, wire, calibrate and read a first sensor.
- [API](api.md): every public method.
- [Sensors](sensors.md): which sensors and gases have fitted curves, and what each sensor needs.
- [Examples](examples.md): what each example sketch shows.
- [Migrating from 1.x](migration.md): the 2.0 replacement for each 1.x call.

The library began as a fork of [MQSensorsLib](https://github.com/miguel5612/MQSensorsLib) by Miguel Califa, Yersson Carrillo and Ghiordy Contreras, and is developed independently since its version 3.0.5. It is MIT licensed.
