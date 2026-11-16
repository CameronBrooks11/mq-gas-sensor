# Sensors

The library works with any sensor whose datasheet plots a resistance ratio against concentration. These are the sensors and gases with a fitted curve, from the [Calibration coefficients](coefficients.md#fitted-curves) page, and the examples that read each:

<!-- table: sensors -->

The coefficients page gives each curve's `a` and `b`, the concentration range it was fitted over, and how far the fit strays from the datasheet's points. Some curves are flat or noisy on their figure and fit poorly; check the **Largest error** column before relying on one.

## MQ-3: mg/L, not ppm

The MQ-3 datasheet plots alcohol in mg/L, so its curves return mg/L. The value from `readPpm()` is in mg/L, and `printDebug()` still labels it ppm.

## MQ-131: R0/Rs

The MQ-131 datasheet plots R0/Rs rather than Rs/R0, so its curves need

```c++
MQ131.setRatio(MQRatio::R0OverRs);
```

`calibrate` still takes the clean-air ratio as Rs/R0. The figure reads R0/Rs = 12.0 in clean air, so [`MQ-131`](examples.md) calibrates with `1.0 / 12.0`.

## MQ-303A: supply offset

The MQ-303A examples compute Rs from the supply less 0.45 V:

```c++
MQ303.setSupplyOffset(0.45);
```

The offset applies wherever Rs is computed, including during calibration.

## MQ-7 and MQ-309A: heater cycle

These two sensors are read while their heater alternates between a high and a low voltage, rather than at a constant 5 V. The examples switch the heater with a logic-level MOSFET on a PWM pin:

![Heater switched by an IRLZ34N from pin 3](img/MQ-309_MQ-7.PNG)

[`MQ-7`](examples.md) runs 60 s at 5 V then 90 s at about 1.4 V; [`MQ-309A`](examples.md) runs 30 s high then 120 s low. Check the timing and voltages against your sensor's datasheet before relying on them, and read the concentration at the point of the cycle the datasheet specifies.

A sensor powered from two separate supplies works too; the library only reads the output.

## ESP32 and ESP8266

The sensor runs from 5 V and its output can reach 5 V, above what a 3.3 V analog input accepts. Divide it down. With 1.7 kΩ over 3.3 kΩ, 5 V becomes 3.3 V:

![An ESP8266 reading an MQ sensor through a 1.7 kΩ / 3.3 kΩ divider](img/MQ_ESP8266.PNG)

The divider scales the output and the supply by the same factor, so leave the supply at its default, the ADC reference (3.3 V), as the ESP examples do. See [Set up the circuit](getting-started.md#set-up-the-circuit).

On the ESP32, Wi-Fi takes over the second ADC (ADC2, GPIOs 0, 2, 4, 12 to 15 and 25 to 27), and analog reads on those pins fail while it is on. Use an ADC1 pin (GPIO 32 to 39) in a sketch that uses Wi-Fi; [`ESP32/ESP32_WROOM_32D`](examples.md) reads GPIO 36 for that reason.

## A sensor not listed

Any MQ-style sensor works the same way: find its sensitivity figure, read points off the curve for the gas you need, and fit `a` and `b`. [Curve tracing](../analysis/README.md) does this reproducibly for a datasheet in the repository's [manifest](../datasheets/README.md); [Methods](methods.md) explains the steps. A curve from another source can be passed to `setCurve` directly.
